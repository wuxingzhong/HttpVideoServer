#pragma once
#include "stdafx.h"

class CHttpSer
{
private:
	uint32_t m_ipaddr;				//�󶨵�ip��ַ

	int32_t m_port;					//�󶨵Ķ˿ں�

	int32_t m_server_skt;			//������socket
	
	int32_t m_isserver_startup;		//��־�������Ƿ�����

	/* ����server_skt */
	int32_t server_skt_create( ); 

	/* ����server_skt */
	void server_skt_destroy( );

	/* �󶨽��յ�ַ�Ͷ˿� */
	int32_t httpser_bind();

	/* ��ʼ����, maxΪ���Ҷ��г��ȣ�����Ϊ5 */
	int32_t httpser_listen(int32_t max = 5);

public:

	/*���캯��  */
	CHttpSer(const char_t* ipaddr = NULL, int32_t port = 8080 );

	/* �ȴ�Զ������ �ɹ�����Զ�̿ͻ���skt ʧ�ܷ��� -1 */
	int32_t httpser_accept();
	
	/*���������*/
	int32_t startup();

	/*�߳����к���*/
	void run(void* expend);

	/*��������*/
	~CHttpSer(void);

public:
	/*�̻߳ص�����*/
	static int32_t server_proc( void* param, void* expend );
};
