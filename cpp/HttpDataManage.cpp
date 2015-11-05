#include "HttpDataManage.h"
#include "osl_file.h"
#define MAX_BUFFER_LEN 128
CHttpDataManage::CHttpDataManage(char_t* config_file_path)
{
	config_file_parser(config_file_path);
}

CHttpDataManage::~CHttpDataManage(void)
{
	while( 0 < m_httpdata_list.size() )
	{
		m_httpdata_list.pop_back();
	}
}


/*�����ļ�����  (δ��fopen win32 ���� FILE linux ���� int fd)*/
void CHttpDataManage::config_file_parser(char_t* config_file_path)
{
	//int32_t ret ; 
	char_t buf[MAX_BUFFER_LEN];
	int32_t bufsize = sizeof(buf);

	if(config_file_path == NULL)
	{
		goto ERROR_EXIT; 
	}

	goto END_EXIT; 
END_EXIT:
	return;
ERROR_EXIT:
	return;

}

/*���һ��http_client�� ��Դ�б�*/
void CHttpDataManage::add_http_client(CHttpClient* phttp_client)
{
	m_httpdata_list.push_back(*phttp_client);
}

/*��ȡ��Դ�б�*/
std::list<CHttpClient>* CHttpDataManage::get_httpdata_list()
{
	return &m_httpdata_list;
}

/*���������� ��ȡhttpclient������*/
void CHttpDataManage::run(void* expend)
{
	int32_t ret;
	
	std::list<CHttpClient>::iterator it;
	it = m_httpdata_list.begin();
	while (it != m_httpdata_list.end())
	{	
		//��buf ����-1�˳�
		ret = CHttpClient::http_client_proc(&(*it), NULL);
		if (ret == -1)
		{	
			it = m_httpdata_list.erase(it); //�Ƴ�һ��http_client
		}
		else
		{
			it++;
		}
	}
}

/*�̻߳ص�����*/
int32_t CHttpDataManage::datamanage_proc( void* param, void* expend )
{
	int32_t ret = -1; 
	if(param == NULL)
	{
		goto ERROR_EXIT;
	}

	CHttpDataManage* p_manage = ( CHttpDataManage* )param; 
	//����
	p_manage->run(expend);
	goto END_EXIT;

END_EXIT:
	return 0;
ERROR_EXIT:
	return -1;
}