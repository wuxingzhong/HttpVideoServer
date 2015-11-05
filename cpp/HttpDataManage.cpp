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


/*配置文件解析  (未完fopen win32 返回 FILE linux 返回 int fd)*/
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

/*添加一个http_client到 资源列表*/
void CHttpDataManage::add_http_client(CHttpClient* phttp_client)
{
	m_httpdata_list.push_back(*phttp_client);
}

/*获取资源列表*/
std::list<CHttpClient>* CHttpDataManage::get_httpdata_list()
{
	return &m_httpdata_list;
}

/*管理器运行 获取httpclient的数据*/
void CHttpDataManage::run(void* expend)
{
	int32_t ret;
	
	std::list<CHttpClient>::iterator it;
	it = m_httpdata_list.begin();
	while (it != m_httpdata_list.end())
	{	
		//读buf 返回-1退出
		ret = CHttpClient::http_client_proc(&(*it), NULL);
		if (ret == -1)
		{	
			it = m_httpdata_list.erase(it); //移除一个http_client
		}
		else
		{
			it++;
		}
	}
}

/*线程回调函数*/
int32_t CHttpDataManage::datamanage_proc( void* param, void* expend )
{
	int32_t ret = -1; 
	if(param == NULL)
	{
		goto ERROR_EXIT;
	}

	CHttpDataManage* p_manage = ( CHttpDataManage* )param; 
	//运行
	p_manage->run(expend);
	goto END_EXIT;

END_EXIT:
	return 0;
ERROR_EXIT:
	return -1;
}