#pragma once
#include "stdafx.h"
#include <list>
#include "Circlebuffer.h"
#include "HttpClient.h"

struct CClientInfo
{
	int32_t client_skt;			//�ͻ���socket

	int32_t read_offset;		//��ȡbufλ��ƫ����

	int32_t flag_send_head;		//�Ƿ�����ͷ

	CHttpClient* p_http_client;	//http��Դ
};

class CClientManage
{
private:

	/*������Ϣ CClientInfo �б�*/
	std::list<CClientInfo> m_send_list;

public:

	CClientManage(void);

	~CClientManage(void);

	/*���һ��client_info�� send_list�б�*/
	void add_client_skt(CClientInfo* pclient_info);
	
	/*����buf���� ����-1 �ѿͻ����˳�*/
	int32_t send_buf(CCirclebuffer* pcir_buf,CClientInfo*  pclient_info);

	/*����������*/
	void run(void* expend);

	/*�̻߳ص�����*/
	static int32_t manage_proc( void* param, void* expend );

};
