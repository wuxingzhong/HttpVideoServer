#include "HttpClient.h"
#include "osl_socket.h"
#include "osl_url.h"
#include "osl_network.h"
#include "osl_log.h"
#include "osl_string.h"
#include "osl.h"
#include "osl_file.h"
#include "Circlebuffer.h"

CHttpClient::CHttpClient(const char_t* url, int32_t cir_buf_len):m_cir_buf(cir_buf_len)
{	
	/*初始化*/
	memset(m_hostname, 0, sizeof(m_hostname));	//主机名
	memset(m_pathdata, 0, sizeof(m_pathdata));	//资源路径和参数
	m_ipaddr = 0;
	m_iscut_head = false;
	m_isclient_startup = false;
	/*url解析*/
	url_parse( url );
}

/*url解析 0表示分析成功,-1分析失败 */
int32_t CHttpClient::url_parse( const char_t* url)
{
	int32_t ret; 
	SUrlInfo info;
	char_t port[8];
	char_t* p_url = ( char_t* )url;

	/* 分析URL,0表示分析成功,-1分析失败 */
	ret = osl_url_parse(p_url, &info );
	if(ret == -1 )
	{
		osl_log_error( "%s[%d], osl_url_parse failed !\n", __FILE__, __LINE__ );
		goto ERROR_EXIT;
	}

	memcpy( m_hostname, url + info.hostname_pos, info.hostname_len ); 
	memcpy( port, p_url + info.hostport_pos, info.hostport_len );  
	osl_strncpy( m_pathdata, p_url + info.path_pos, sizeof(m_pathdata)-1 ); 
	m_port = atoi(port);
	goto END_EXIT;

END_EXIT:
	return 0;
ERROR_EXIT:
	return -1;
}

/*DNS解析, hostname->IP地址 */
void CHttpClient::url_network_dns()
{
	osl_log_info( "url_network_dnsn...\n");
	m_ipaddr = osl_network_dns( m_hostname );
}

/* 创建client_skt */
int32_t CHttpClient::client_skt_create( )
{
	int32_t ret;
	/*创建socket*/
	m_client_skt = osl_socket_create( AF_INET, SOCK_STREAM, IPPROTO_TCP ); 
	if(m_client_skt == -1)
	{
		osl_log_error( "%s[%d], create client failed!\n", __FILE__, __LINE__ );
		goto ERROR_EXIT;
	}
	/* 非0 开启io非阻塞 */
	uint32_t  imode = 1; 
	ret = osl_socket_ioctl(m_client_skt, FIONBIO, &imode);
	if(ret != 0 )
	{
		osl_log_error( "%s[%d], osl_socket_ioctl failed!\n", __FILE__, __LINE__ );
		goto ERROR_EXIT;
	}
	goto END_EXIT;

END_EXIT:
	return 0;
ERROR_EXIT:
	return -1;
}

/* 销毁client_skt */
void CHttpClient::client_skt_destroy( )
{
	osl_socket_destroy( m_client_skt );
}


/*连接 返回0 表示成功, -1 表示失败*/
int32_t CHttpClient::url_connect()
{	
	int32_t ret;
	/* 连接远端 */
	do
	{	
		osl_socket_connect( m_client_skt, m_ipaddr, htons(m_port));
		ret = osl_socket_get_connect_state( m_client_skt );
		if(ret == 0 )
		{
			/* 0 表示连接成功*/
			osl_log_info( "%s[%d]:\nosl_socket_connect ok ! !\n", __FILE__, __LINE__ );
			break;
		}
		else if (ret == 1)
		{	 
			/*1 表示等待 */ 
			osl_usleep(500000);
		}
		else if(ret == -1)
		{	
			/*-1 表示错误，应关闭*/
			osl_log_error( "%s[%d], osl_socket_connect failed ! \n", __FILE__, __LINE__ );
			goto ERROR_EXIT;
		}
	}while(1);

	goto END_EXIT;

END_EXIT:
	return 0;
ERROR_EXIT:
	return -1;

}

/*发送http请求 返回0 表示成功, -1 表示失败 */
int32_t  CHttpClient::url_send(const char_t* pathdata)
{
	char_t send_str[1024] = { 0 };
	int32_t size,ret, tmp = 0;
	size = sizeof(send_str);

	/* 资源路径 如果未指定则为默认的路径*/
	char_t* p_pathdata = m_pathdata; 
	if(pathdata != NULL)
	{	/* 资源路径 修改为指定的路径*/
		p_pathdata = (char_t* )pathdata ;
	}

	/*发送http请求头*/
	//封装头信息
	osl_str_snprintf(send_str, sizeof(send_str) -1, 
		"GET %s HTTP/1.1\r\n"
		"Host: %s\r\n"
		"Accept: */*\r\n"
		"Connection: Keep-Alive\r\n"
		"User-Agent: Mozilla/5.0 (compatible; Qteqpidspider/1.0;)\r\n"
		"Referer: %s\r\n\r\n", 
		p_pathdata, m_hostname, m_hostname );
	/* 发送数据 */
	while( 0 < size ) 
	{
		ret = osl_socket_send( m_client_skt, send_str+tmp, size );
		
		if( 0 < ret ) 
		{
			size -= ret;
			tmp += ret;
			osl_log_info( "osl_socket_send ret = %d! \n",ret);
		}
		else if( 0 == ret)
		{
			 osl_log_error( "%s[%d], osl_socket_send server exit ! \n", __FILE__, __LINE__ );
			 goto ERROR_EXIT;
		}
		else if( ret  < 0 )
		{
			
			/*0=正常  -1=socket出错*/
			if(osl_socket_get_state() == -1)
			{
				osl_log_error( "osl_socket_send ret = %d! \n",ret);
				goto ERROR_EXIT;
			}
				
		}
	}
	goto END_EXIT;

END_EXIT:
	m_iscut_head = false ; //是否已经去除http报文头
	return 0;
ERROR_EXIT:
	return -1;
}

/*接收http*/
int32_t CHttpClient::url_recv(char_t* recv_buf, int32_t size)
{	
	int32_t ret;
	char_t*  pstr = NULL;
	char_t code[4] = { 0 }; 
	/* 接收数据 */
	 ret = osl_socket_recv( m_client_skt, recv_buf, size );
	  
	 if( ret  == -1 )
	 {	
		 return ret;
	 }
	 else if( ret  < -1 )
	 {
		 /*0=正常  -1=socket出错*/
		if(osl_socket_get_state() == -1)
		{
			ret = -5;
		}
	 } 
	 else if( 0 == ret)
	 {
		 osl_log_error( "%s[%d], osl_socket_recv server exit ! \n", __FILE__, __LINE__ );
	 }
	 //else if( 0 < ret ) 
	 //{
		//
		// /* 是否已经去除http报文头*/
		//	
		// if(m_iscut_head == false )
		// {
		//	  printf("%s======", recv_buf);
		//	pstr = osl_strstr( recv_buf, "\r\n\r\n");
		//	/*是报文头*/
		//	if(pstr != NULL )
		//	{	
		//		
		//		memcpy(code, recv_buf + 9 , 3);
		//		if (atoi(code) != 200)
		//		{	
		//				ret = -3;
		//		}
		//		/*报文后面是有数据 有则写入recv_buf*/
		//		if( *(pstr + 5) != 0)
		//		{
		//			ret = ret - (recv_buf - pstr - 4);
		//			memcpy( recv_buf, pstr + 4 , ret );
		//		}else
		//		{
		//			ret = -4;
		//		}
		//		m_iscut_head = true; //标志已经去除报文头
		//	}
		// }
	 //}

	 return ret;
}

/*客户端启动*/
int32_t CHttpClient::startup()
{	
	int32_t ret;
	/*DNS解析*/
	url_network_dns();
	ret = client_skt_create( );
	if(ret != 0 )
	{
		osl_log_info( "%s[%d], client_skt_create failed! !\n", __FILE__, __LINE__ );
		goto ERROR_EXIT;
	}

	/*连接 返回0 表示成功, -1 表示失败*/
	ret =  url_connect();
	if(ret == -1)
	{
		goto ERROR_EXIT;
	}
	/*发送http请求*/
	ret = url_send();
	if(ret == -1)
	{
		goto ERROR_EXIT;
	}

	goto END_EXIT;

END_EXIT:
	return 0;
ERROR_EXIT:
	return -1;

}

/*客户端运行*/
void CHttpClient::run(void* expend)
{	
	int32_t ret;
	/*接收http返回*/ 
	int count = 4 ; //接收数据次数
	//if(expend == NULL)
	//{
	//	osl_log_error( "%s[%d], expend == NULL! \n", __FILE__, __LINE__ );
	//	return ;
	//}
	//else
	{
		//写buf
		while(count)
		{
			/* 接收数据 */
			ret = url_recv((char_t*)m_cir_buf.get_write_pos(), m_cir_buf.get_curmax_write_len());
			if( 0 < ret ) 
			{	 
				/* 写入成功 移动buf 尾部 移动长度为ret */
				m_cir_buf.move_curtail(ret);
			}
			else if( 0 == ret)
			{
				osl_log_error( "%s[%d], osl_socket_recv server exit ! \n", __FILE__, __LINE__ );
				return ;
			}
			else if( ret == -5 )
			{
				osl_log_error( "%s[%d], osl_socket_recv failed ! \n", __FILE__, __LINE__ );
				return ;
			}
			count -- ;
		}
	}
}

CHttpClient::~CHttpClient(void)
{
	/* 销毁socket */
	client_skt_destroy( );
}

/*获取环形buf*/
CCirclebuffer* CHttpClient::get_cir_buf()
{
	return &m_cir_buf;
}

/*获取资源路径*/
char_t* CHttpClient::get_pathdata()
{
	return m_pathdata;
}

int32_t CHttpClient::http_client_proc( void* param, void* expend )
{
	int32_t ret; 
	if(param == NULL)
	{
		goto ERROR_EXIT;
	}
	CHttpClient* p_client = ( CHttpClient* )param; 
	/* 启动客户端 */
	if(!p_client->m_isclient_startup)
	{
		ret = p_client->startup();
		if(ret == -1)
		{
			goto ERROR_EXIT;
		}else
		{
			p_client->m_isclient_startup = true; //标志该客户端已经启动
		}
	}
	//客户端运行
	p_client->run(expend);
	goto END_EXIT;
END_EXIT:
	return 0;
ERROR_EXIT:
	return -1;
}

/* UTC->DATE( foamat) */
char_t* CHttpClient::osl_str_utcfomat( uint32_t utc, const char_t *format, char_t *buf, int32_t size )
{
	struct tm *tt;
	time_t time_utc = utc;

	tt = localtime( &time_utc );
	if( tt == NULL )
	{
		buf[0] = 0;
	}
	else
	{
#ifdef WIN32
		_snprintf( buf, size, format, tt->tm_year+1900, tt->tm_mon+1, tt->tm_mday,tt->tm_hour,tt->tm_min,tt->tm_sec );
#else
		snprintf( buf, size, format,  tt->tm_year+1900, tt->tm_mon+1, tt->tm_mday,tt->tm_hour,tt->tm_min,tt->tm_sec );
#endif
	}
	return buf;
}
