#pragma once
#include "stdafx.h"
#include "Circlebuffer.h"
#define DEFAULT_BUFFER_LEN 1024*1024*3

class CHttpClient
{
private:
	char_t m_hostname[64] ;			//������ 

	int32_t m_port;					//�˿ں�

	char_t m_pathdata[64];			//��Դ·���Ͳ���

	int32_t  m_ipaddr;				//����IP��ַ

	int32_t m_client_skt;			// �ͻ���socket
	
	int32_t m_iscut_head ;			//�Ƿ��Ѿ�ȥ��http����ͷ
	
	int32_t m_isclient_startup;		//�ÿͻ����Ƿ��Ѿ�����
	
	CCirclebuffer m_cir_buf;		//����buf;
	
	/*url����  0��ʾ�����ɹ�,-1����ʧ��*/
	int32_t url_parse( const char_t* url);

	/*DNS����, hostname->IP��ַ */
	void url_network_dns();

	/* ����client_skt */
	int32_t client_skt_create( ); 

	/* ����client_skt */
	void client_skt_destroy( );

	/*���� ����0 ��ʾ�ɹ�, -1 ��ʾʧ��*/
	int32_t url_connect();

	/*�ͻ�������*/
	int32_t startup();

	/*�ͻ�������*/
	void run(void* expend);

public:

	/*���캯�� ����һ��url ��buf�ĳ���*/
	CHttpClient( const char_t* url,  int32_t cir_buf_len = DEFAULT_BUFFER_LEN );

	/*��ȡ����buf*/
	CCirclebuffer* get_cir_buf();

	/*��ȡ��Դ·��*/
	char_t* get_pathdata();
	
	/*����http���� ����0 ��ʾ�ɹ�, -1 ��ʾʧ�� */
	int32_t url_send(const char_t* pathdata = NULL);

	/*����http*/
	int32_t url_recv(char_t* recv_buf, int32_t size);

	/*����*/
	~CHttpClient( void );

public:

	/* UTC->DATE( foamat) */
	static char_t *osl_str_utcfomat( uint32_t utc, const char_t *format, char_t *buf, int32_t size );

	/*�̻߳ص����� param ����ָ��CHttpClient �Ķ����ָ��*/
	static int32_t http_client_proc( void* param, void* expend );
};
