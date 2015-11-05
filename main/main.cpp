#include "stdafx.h"
#include "osl_string.h"
#include "HttpClient.h"
#include "osl_log.h"
#include "osl.h"
#include "osl_socket.h"
#include "HttpSer.h"
#include "osl_thread.h"
#include "ClientManage.h"
#include "HttpDataManage.h"


int main()
{
	int32_t ret;

	osl_log_set_target( "console://" );
	char_t *p_url  = "http://139.196.46.132:5006/football_bq.ts?token=guoziyun";
	//char_t *p_url2 = "http://139.196.46.132:5006/football_gq.ts?token=guoziyun";
//	char_t *p_url3 = "http://139.196.46.132:5006/football_cq.ts?token=guoziyun";
	
	/*数据源*/
	CHttpClient h_client(p_url);
	//CHttpClient h_client2(p_url2);
	//CHttpClient h_client3(p_url3);
	/*源管理器*/
	CHttpDataManage data_manage("这里可以放视频源配置文件");
	/*添加源*/ 
	data_manage.add_http_client(&h_client);
	//data_manage.add_http_client(&h_client2);
	//data_manage.add_http_client(&h_client3);  
	
	/*http监听服务器*/
	CHttpSer http_server;

	/*客户端管理器*/
	CClientManage client_manage;

	/* 初始化资源 */
	ret = osl_init( "123");
	if( ret == -1 )
	{
		osl_log_error( "%s[%d], osl_init failed!\n", __FILE__, __LINE__ );
		goto ERROR_EXIT;
	}
	//开线程式 
	//void* pthread_client = osl_thread_create( "h_client", THREAD_DEFAULT_PRIORIOTY, THREAD_DEFAULT_STACK_SIZE,
	//	CHttpDataManage::datamanage_proc, &data_manage, NULL);
	//osl_thread_start(pthread_client);

	//osl_usleep(5000000);  //给获取数据的时间

	//void* pthread_server = osl_thread_create( "http_server", 0, THREAD_DEFAULT_STACK_SIZE,
	//	CClientManage::manage_proc, &client_manage, &data_manage);
	//osl_thread_start(pthread_server);
	
	//单个线程式
	do 
	{	
		//运行 http数据源管理器
		CHttpDataManage::datamanage_proc(&data_manage, NULL);

		//运行 http监听服务器
		CHttpSer::server_proc(&http_server, &client_manage);

		//运行 客户端管理器
		CClientManage::manage_proc(&client_manage, &data_manage);
	}while(1);

	getchar();
	/* 销毁线程 强制销毁，返回0表示成功,-1表示失败 */
	//ret = osl_thread_destroy( pthread_client, 300);
	//ret = osl_thread_destroy( pthread_server, 300);
	//if( ret == -1)
	//{
	//	printf("osl_thread_destroy failed!!\n");
	//}

	printf("press any key to continue!!\n");
	goto END_EXIT;
END_EXIT:
	getchar();
	return 0;
ERROR_EXIT:
	getchar();
	return -1;
}