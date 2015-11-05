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
	/*初始化*/
	m_header = NULL;			// buffer的开始位置
	m_tail = NULL;				//buffer的结束位置
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
	m_flag_cir = 0 ;	// 标志环形buf是否已经写满, 开始覆盖式写入 false 未写满
}

CCirclebuffer::~CCirclebuffer(void)
{
	//释放内存
	delete[] m_header ;
}

/*获取buffer的长度*/
int32_t CCirclebuffer::get_buflen()
{
	return m_buflen;
}

/*获取开始写buf的位置, (即m_curtail+1 的位置或者m_header)*/
void* CCirclebuffer::get_write_pos()
{
	if( m_curtail == m_tail )
	{
		m_flag_cir = true;
		return m_header;
	}

	return m_curtail + 1 ; 
}

/*获取本次最大可写长度*/
int32_t CCirclebuffer::get_curmax_write_len()
{
	if( m_curtail == m_tail )
	{
		return get_buflen();
	}

	return m_tail - m_curtail;
}

/*移动当前末尾的指针, 返回实际移动的长度*/
int32_t CCirclebuffer::move_curtail(int32_t move_len)
{
	int32_t max = get_curmax_write_len();

	//进行修正
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

/*获取m_header*/
void* CCirclebuffer::get_header()
{
	return m_header;
}

/*获取第一次开始读的位置距m_header的偏移量*/
int32_t CCirclebuffer::get_begin_read_offset()
{	
	int32_t offset = 0 ;
	int32_t less_len = 0;
	//每次都从环形buf 的中间段开始读取
	//环形buf未写满
	if( !m_flag_cir ) 
	{
		offset = (m_curtail - m_header)/2; 
	}
	else
	{	
		less_len = m_tail - m_curtail;
		if( (m_buflen/2) <= less_len)
		{
			//尾部足够
			offset = (m_curtail -m_header) + m_buflen/2 ; 
		}
		else
		{
			//从头部开始
			offset =  (m_buflen/2 - less_len);
		}
	}

	return offset;
}

/*根据偏移量获取读位置*/
void* CCirclebuffer::get_read_pos(int32_t offset_len)
{
	int32_t max = get_buflen() - 1 ;
	//修正
	if(max < offset_len )
	{
		offset_len = max;
	}
	return m_header + offset_len;
}

/*获取本次最大可读长度*/
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
	{	//过大 发送 1/10
		len = len /10;
	}
	return len ;
	
}

/*移动读位置的偏移量*/
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