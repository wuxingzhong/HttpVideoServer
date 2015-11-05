#pragma once
#include "stdafx.h"
#include "Circlebuffer.h"
#define DEFAULT_BUFFER_LEN 1024*1024*3

class CHttpClient
{
private:
	char_t m_hostname[64] ;			//主机名 

	int32_t m_port;					//端口号

	char_t m_pathdata[64];			//资源路径和参数

	int32_t  m_ipaddr;				//主机IP地址

	int32_t m_client_skt;			// 客户端socket
	
	int32_t m_iscut_head ;			//是否已经去除http报文头
	
	int32_t m_isclient_startup;		//该客户端是否已经启动
	
	CCirclebuffer m_cir_buf;		//环形buf;
	
	/*url解析  0表示分析成功,-1分析失败*/
	int32_t url_parse( const char_t* url);

	/*DNS解析, hostname->IP地址 */
	void url_network_dns();

	/* 创建client_skt */
	int32_t client_skt_create( ); 

	/* 销毁client_skt */
	void client_skt_destroy( );

	/*连接 返回0 表示成功, -1 表示失败*/
	int32_t url_connect();

	/*客户端启动*/
	int32_t startup();

	/*客户端运行*/
	void run(void* expend);

public:

	/*构造函数 传入一个url 和buf的长度*/
	CHttpClient( const char_t* url,  int32_t cir_buf_len = DEFAULT_BUFFER_LEN );

	/*获取环形buf*/
	CCirclebuffer* get_cir_buf();

	/*获取资源路径*/
	char_t* get_pathdata();
	
	/*发送http请求 返回0 表示成功, -1 表示失败 */
	int32_t url_send(const char_t* pathdata = NULL);

	/*接收http*/
	int32_t url_recv(char_t* recv_buf, int32_t size);

	/*析构*/
	~CHttpClient( void );

public:

	/* UTC->DATE( foamat) */
	static char_t *osl_str_utcfomat( uint32_t utc, const char_t *format, char_t *buf, int32_t size );

	/*线程回调函数 param 传入指向CHttpClient 的对象的指针*/
	static int32_t http_client_proc( void* param, void* expend );
};
