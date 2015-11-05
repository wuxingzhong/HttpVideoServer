#pragma once
#include <list>
#include "HttpClient.h"

class CHttpDataManage
{
private:

	/*资源列表*/
	std::list<CHttpClient> m_httpdata_list;

public:

	CHttpDataManage(char_t* config_file_path = NULL);

	~CHttpDataManage(void);
	
	/*配置文件解析*/
	 void config_file_parser(char_t* config_file_path);

	/*添加一个http_client到 资源列表*/
	void add_http_client(CHttpClient* phttp_client);
	
	/*获取资源列表*/
	std::list<CHttpClient>* get_httpdata_list();

	/*管理器运行*/
	void run(void* expend);

	/*线程回调函数*/
	static int32_t datamanage_proc( void* param, void* expend );
};
