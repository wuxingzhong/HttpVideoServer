#include "Circlebuffer.h"
#include "osl_socket.h"
#include "osl_url.h"
#include "osl_network.h"
#include "osl_log.h"
#include "osl_string.h"
#include "osl.h"
#define MAX_SIZE 1024*40

CCirclebuffer::CCirclebuffer(int32_t  buf_len)
{
	/*��ʼ��*/
	m_header = NULL;			// buffer�Ŀ�ʼλ��
	m_tail = NULL;				//buffer�Ľ���λ��
	m_curtail = m_header;
	m_buflen = 0 ; 

	m_header = new char_t[buf_len];
	if(m_header == NULL)
	{
		osl_log_error( "%s[%d], CCirclebuffer malloc failed buf_len = %d!\n", __FILE__, __LINE__, buf_len );
		return ;
	}

	m_buflen = buf_len;
	memset(m_header, 0, buf_len);
	m_tail = m_header + buf_len -1 ;
	m_curtail = m_header -1 ;
	m_flag_cir = 0 ;	// ��־����buf�Ƿ��Ѿ�д��, ��ʼ����ʽд�� false δд��
}

CCirclebuffer::~CCirclebuffer(void)
{
	//�ͷ��ڴ�
	delete[] m_header ;
}

/*��ȡbuffer�ĳ���*/
int32_t CCirclebuffer::get_buflen()
{
	return m_buflen;
}

/*��ȡ��ʼдbuf��λ��, (��m_curtail+1 ��λ�û���m_header)*/
void* CCirclebuffer::get_write_pos()
{
	if( m_curtail == m_tail )
	{
		m_flag_cir = true;
		return m_header;
	}

	return m_curtail + 1 ; 
}

/*��ȡ��������д����*/
int32_t CCirclebuffer::get_curmax_write_len()
{
	if( m_curtail == m_tail )
	{
		return get_buflen();
	}

	return m_tail - m_curtail;
}

/*�ƶ���ǰĩβ��ָ��, ����ʵ���ƶ��ĳ���*/
int32_t CCirclebuffer::move_curtail(int32_t move_len)
{
	int32_t max = get_curmax_write_len();

	//��������
	if ( max < move_len)
	{
		move_len = max ; 
	}
	if( m_curtail == m_tail )
	{
		move_len = 0;
		m_curtail = m_header;
	}
	m_curtail += move_len;

	return move_len;
}

/*��ȡm_header*/
void* CCirclebuffer::get_header()
{
	return m_header;
}

/*��ȡ��һ�ο�ʼ����λ�þ�m_header��ƫ����*/
int32_t CCirclebuffer::get_begin_read_offset()
{	
	int32_t offset = 0 ;
	int32_t less_len = 0;
	//ÿ�ζ��ӻ���buf ���м�ο�ʼ��ȡ
	//����bufδд��
	if( !m_flag_cir ) 
	{
		offset = (m_curtail - m_header)/2; 
	}
	else
	{	
		less_len = m_tail - m_curtail;
		if( (m_buflen/2) <= less_len)
		{
			//β���㹻
			offset = (m_curtail -m_header) + m_buflen/2 ; 
		}
		else
		{
			//��ͷ����ʼ
			offset =  (m_buflen/2 - less_len);
		}
	}

	return offset;
}

/*����ƫ������ȡ��λ��*/
void* CCirclebuffer::get_read_pos(int32_t offset_len)
{
	int32_t max = get_buflen() - 1 ;
	//����
	if(max < offset_len )
	{
		offset_len = max;
	}
	return m_header + offset_len;
}

/*��ȡ�������ɶ�����*/
int32_t CCirclebuffer::get_curmax_read_len(void* read_pos)
{	
	int len = 0 ;
	if(!m_flag_cir)
	{
		len =  m_curtail - ( char_t* )read_pos + 1;
	}else
	{
		if(( char_t* )read_pos >= m_curtail )
		{
			len =  m_tail - ( char_t* )read_pos + 1;  
		}else
		{
			len =  m_curtail - ( char_t* )read_pos + 1;
		}
	}
	if( MAX_SIZE < len)
	{	//���� ���� 1/10
		len = len /10;
	}
	return len ;
	
}

/*�ƶ���λ�õ�ƫ����*/
int32_t CCirclebuffer::move_read_offset(int32_t* pread_offset, int32_t offset_len)
{

	int32_t max = get_curmax_read_len( get_read_pos(*pread_offset) );
	if(max < offset_len)
	{	
		offset_len = max;
	}

	*pread_offset += offset_len;
	if(*pread_offset == m_buflen )
	{
		*pread_offset = 0 ;
	}
	osl_log_info("*pread_offse == %d\n", *pread_offset);
	return offset_len;
}