#pragma once
#include "stdafx.h"
#include <list>
#include "Circlebuffer.h"
#include "HttpClient.h"

struct CClientInfo
{
	int32_t client_skt;			//客户端socket

	int32_t read_offset;		//读取buf位置偏移量

	int32_t flag_send_head;		//是否发送了头

	CHttpClient* p_http_client;	//http资源
};

class CClientManage
{
private:

	/*发送消息 CClientInfo 列表*/
	std::list<CClientInfo> m_send_list;

public:

	CClientManage(void);

	~CClientManage(void);

	/*添加一个client_info到 send_list列表*/
	void add_client_skt(CClientInfo* pclient_info);
	
	/*发送buf数据 返回-1 把客户端退出*/
	int32_t send_buf(CCirclebuffer* pcir_buf,CClientInfo*  pclient_info);

	/*管理器运行*/
	void run(void* expend);

	/*线程回调函数*/
	static int32_t manage_proc( void* param, void* expend );

};
