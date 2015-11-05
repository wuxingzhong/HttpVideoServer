#pragma once
#include <list>
#include "HttpClient.h"

class CHttpDataManage
{
private:

	/*��Դ�б�*/
	std::list<CHttpClient> m_httpdata_list;

public:

	CHttpDataManage(char_t* config_file_path = NULL);

	~CHttpDataManage(void);
	
	/*�����ļ�����*/
	 void config_file_parser(char_t* config_file_path);

	/*���һ��http_client�� ��Դ�б�*/
	void add_http_client(CHttpClient* phttp_client);
	
	/*��ȡ��Դ�б�*/
	std::list<CHttpClient>* get_httpdata_list();

	/*����������*/
	void run(void* expend);

	/*�̻߳ص�����*/
	static int32_t datamanage_proc( void* param, void* expend );
};
