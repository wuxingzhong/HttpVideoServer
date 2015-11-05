#include "HttpSer.h"
#include "osl_socket.h"
#include "osl_url.h"
#include "osl_network.h"
#include "osl_log.h"
#include "osl_string.h"
#include "osl.h"
#include "HttpClient.h"
#include "ClientManage.h"

/*构造函数 */
CHttpSer::CHttpSer(const char_t* ipaddr, int32_t port)
{
	if(ipaddr == NULL)
	{
		m_ipaddr = INADDR_ANY;
	}
	else
	{
		m_ipaddr = inet_addr(ipaddr);
	}
	m_port = port;	
	m_isserver_startup = false;

}

/* 创建server_skt */
int32_t CHttpSer::server_skt_create( )
{
	int32_t ret;
	/*创建socket*/
	m_server_skt = osl_socket_create( AF_INET, SOCK_STREAM, IPPROTO_TCP ); 
	if(m_server_skt == -1)
	{
		osl_log_error( "%s[%d], create client failed!\n", __FILE__, __LINE__ );
		goto ERROR_EXIT;
	}
	/* 非0 开启io非阻塞 */
	uint32_t  imode = 1; 
	ret = osl_socket_ioctl(m_server_skt, FIONBIO, &imode);
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

/* 销毁server_skt */
void CHttpSer::server_skt_destroy( )
{
	osl_socket_destroy( m_server_skt );
}

/* 绑定接收地址和端口 */
int32_t CHttpSer::httpser_bind()
{
	return osl_socket_bind(m_server_skt, m_ipaddr, htons(m_port));
}

/* 开始监听, max为悬挂队列长度，建议设为5 */
int32_t CHttpSer::httpser_listen(int32_t max )
{
	return osl_socket_listen( m_server_skt, max);
}

/* 等待远程连接 成功返回远程客户端skt  失败返回 -1  */
int32_t CHttpSer::httpser_accept()
{
	/* 等待远程连接 */
	uint32_t ip;
	uint16_t port;
	return osl_socket_accept( m_server_skt, &ip, &port); 
}


/*服务端启动*/
int32_t CHttpSer::startup()
{
	int32_t ret;

	/* 创建server_skt */
	ret = server_skt_create( );
	if(ret == -1)
	{
		goto ERROR_EXIT;
	}

	/* 绑定server_skt */
	ret = httpser_bind( );
	if(ret == -1)
	{
		goto ERROR_EXIT;
	}

	/* 开始监听, max为悬挂队列长度，默认设5 */
	ret = httpser_listen();
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

CHttpSer::~CHttpSer(void)
{
	server_skt_destroy( );
}

void CHttpSer::run(void* expend)
{
	
	if(expend == NULL)
	{
		osl_log_error( "%s[%d], expend == NULL! \n", __FILE__, __LINE__ );
		return ;
	}
	 
	CClientManage* p_client_manage = (CClientManage*)expend;
	int32_t client_skt;
	CClientInfo info ={ 0 };  // 初始化
	info.read_offset = -1;
	info.p_http_client = NULL; // 读取资源设置为NULL
	client_skt = httpser_accept();
	if(client_skt != -1)
	{	
		printf("client_skt = %d \n", client_skt);
		info.client_skt = client_skt;
		p_client_manage->add_client_skt(&info);
	}
}

/*线程回调函数*/
int32_t CHttpSer::server_proc( void* param, void* expend )
{
	int32_t ret; 
	if(param == NULL)
	{
		goto ERROR_EXIT;
	}
	CHttpSer* p_server = ( CHttpSer* )param; 
	/* 启动服务器 */
	if(!p_server->m_isserver_startup)
	{
		ret = p_server->startup();
		if(ret == -1)
		{
			goto ERROR_EXIT;
		}else
		{
			p_server->m_isserver_startup = true; //标志该服务器已经启动
			/* 等待远程连接 成功返回远程客户端skt  */
			osl_log_info( "httpser_accept!!\n") ;
		}
	}

	//服务器运行
	p_server->run(expend);

	goto END_EXIT;
END_EXIT:
	return 0;
ERROR_EXIT:
	return -1;
}