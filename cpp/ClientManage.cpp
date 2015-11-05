#include "Circlebuffer.h"
#include "osl_socket.h"
#include "osl_url.h"
#include "osl_network.h"
#include "osl_log.h"
#include "osl_string.h"
#include "osl.h"
#include "ClientManage.h"
#include "Circlebuffer.h"
#include "HttpDataManage.h"
#include <list>

CClientManage::CClientManage(void)
{
	
}

CClientManage::~CClientManage(void)
{
	
	while( 0 < m_send_list.size() )
	{
		CClientInfo info = (CClientInfo)m_send_list.back();
		osl_socket_destroy(info.client_skt);
		m_send_list.pop_back();
		
	}

	
}


/*���һ��client_skt�� send_listt����*/
void CClientManage::add_client_skt(CClientInfo* pclient_info)
{
	m_send_list.push_back(*pclient_info);
}

int32_t CClientManage::send_buf(CCirclebuffer* pcir_buf,CClientInfo*  pclient_info)
{	
	int32_t ret;
	void* read_pos;
	int32_t readsize = 0; 
	int32_t count = 3; //���ʹ���
	char_t *head = 
		"HTTP/1.1 200 OK\r\n"
		"Server: wxz-sever 1.0\r\n"
		"Accept-Ranges: bytes\r\n"
		"Last-Modified: Mar 20 2015\r\n"
		"Content-Type: application/file\r\n"
		"Content-Length: -1\r\n"
		"Connection: keep-alive \r\n\r\n";

	//�Ƿ���Ҫ����ͷ
	if(!pclient_info->flag_send_head)
	{
		printf("send header!!\n");
		osl_socket_send(pclient_info->client_skt, head, strlen(head)) ;
		pclient_info->flag_send_head = 1; //�ѷ���ͷ
	}

	while(count)
	{	
		if( pclient_info->read_offset == -1)
		{
			pclient_info->read_offset = pcir_buf->get_begin_read_offset();
		}

		read_pos = pcir_buf->get_read_pos(pclient_info->read_offset);
		ret = osl_socket_send(pclient_info->client_skt, 
				read_pos, 
				pcir_buf->get_curmax_read_len(read_pos)) ;
		if(ret == -1 )
		{
			if(osl_socket_get_state() == -1)
			{
				printf("client exit!! \n");
				goto ERROR_EXIT;
			}
		}
		else if(ret >0)
		{
			pcir_buf->move_read_offset(&(pclient_info->read_offset), ret);
		}
		count--;
	}
	goto END_EXIT;
END_EXIT:
	return 0;
ERROR_EXIT:
	return -1;
}

/*����������*/
void CClientManage::run(void* expend)
{
	int32_t ret;
	CHttpClient* p_http_client= NULL; 
	if(expend == NULL)
	{
		osl_log_error( "%s[%d], CClientManage::run,expend == NULL! \n", __FILE__, __LINE__ );
		return;
	}
	CHttpDataManage* p_http_data = (CHttpDataManage*)expend;

	std::list<CClientInfo>::iterator it;
	
	it = m_send_list.begin();
	//���ʱ����Ҫselect ����epoll ����list?
	while (it != m_send_list.end())
	{	
		//��buf ����-1�ͻ����˳�
		p_http_client = it->p_http_client ; 
		if(p_http_client == NULL)
		{	
			char_t head_buf[1024];
			int32_t buflen = sizeof(head_buf);

			//Ϊ��ƥ����Դ
			std::list<CHttpClient>* p_data_list =  p_http_data->get_httpdata_list(); 
			
			//��ȡͷ��
			ret = osl_socket_recv(it->client_skt, head_buf, buflen);
			
			if (ret < 0)
			{
				/*0=����  -1=socket����*/
				if(osl_socket_get_state() == -1)
				{
					it = m_send_list.erase(it); //�Ƴ�һ���ͻ���
				}
				continue;
			}
			else if( 0 < ret )
			{	
				char_t*  p_find = NULL ;
				std::list<CHttpClient>::iterator it_data;

				//Ѱ����Դ
				it_data = p_data_list->begin();
				while (it_data != p_data_list->end())
				{	
					//ƥ����Դ·��
					head_buf[ret] = 0;
					p_find = osl_strstr(head_buf, it_data->get_pathdata());
	
					if( p_find != NULL )
					{
						//ƥ��
						it->p_http_client = &*(it_data);
						p_http_client = it->p_http_client ; 
						break;
					}
					it_data++;
				}
					//δƥ�䵽
				if(it_data == p_data_list->end())
				{
					//δ�ҵ���Դ ���client
					osl_socket_destroy(it->client_skt);
					it = m_send_list.erase(it); //�Ƴ�һ���ͻ���
					continue;
				}
			}
			p_http_client = it->p_http_client ; 
		}

		if(p_http_client == NULL)
		{
			it++;
			continue;
		}

		ret = send_buf(p_http_client->get_cir_buf(), &(*it));
		if (ret == -1)
		{	
			osl_socket_destroy(it->client_skt);
			it = m_send_list.erase(it); //�Ƴ�һ���ͻ���
		}
		else
		{
			it++;
		}
	}
	
}


/*�̻߳ص�����*/
int32_t CClientManage::manage_proc( void* param, void* expend )
{
	int32_t ret = -1; 
	if(param == NULL)
	{
		goto ERROR_EXIT;
	}

	CClientManage* p_manage = ( CClientManage* )param; 
	//����
	p_manage->run(expend);
	goto END_EXIT;

END_EXIT:
	return 0;
ERROR_EXIT:
	return -1;
}
