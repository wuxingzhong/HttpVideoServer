#pragma once
#include "stdafx.h"

class CHttpSer
{
private:
	uint32_t m_ipaddr;				//绑定的ip地址

	int32_t m_port;					//绑定的端口号

	int32_t m_server_skt;			//服务器socket
	
	int32_t m_isserver_startup;		//标志服务器是否启动

	/* 创建server_skt */
	int32_t server_skt_create( ); 

	/* 销毁server_skt */
	void server_skt_destroy( );

	/* 绑定接收地址和端口 */
	int32_t httpser_bind();

	/* 开始监听, max为悬挂队列长度，建议为5 */
	int32_t httpser_listen(int32_t max = 5);

public:

	/*构造函数  */
	CHttpSer(const char_t* ipaddr = NULL, int32_t port = 8080 );

	/* 等待远程连接 成功返回远程客户端skt 失败返回 -1 */
	int32_t httpser_accept();
	
	/*服务端启动*/
	int32_t startup();

	/*线程运行函数*/
	void run(void* expend);

	/*析构函数*/
	~CHttpSer(void);

public:
	/*线程回调函数*/
	static int32_t server_proc( void* param, void* expend );
};
