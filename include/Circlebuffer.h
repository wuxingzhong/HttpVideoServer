#pragma once
#include "stdafx.h"
#define DEFAULT_BUFFER_LEN 1024*1024*3
class CCirclebuffer
{
private:
	char_t *m_header;		// buffer的开始位置

	char_t *m_tail;			// buffer的结束位置

	int32_t m_buflen;		// buffer的长度

	char_t *m_curtail;		// 指向环形buf的尾部位置

	int32_t m_flag_cir;		// 标志环形buf是否已经写满, 开始覆盖式写入
	
public:
	
	//默认分配1024*1024*3 = 3MB 
	CCirclebuffer(int32_t buf_len = DEFAULT_BUFFER_LEN );

	~CCirclebuffer(void);
	
	/*获取buffer的长度*/
	int32_t get_buflen();

	/*获取开始写buf的位置, (即m_curtail+1 的位置或者m_header)*/
	void* get_write_pos();

	/*获取本次最大可写长度*/
	int32_t get_curmax_write_len();

	/*移动当前末尾的指针, 返回实际移动的长度*/
	int32_t move_curtail(int32_t move_len);

	/*获取m_header*/
	void* get_header();
	
	/*获取第一次开始读的位置距m_header的偏移量*/
	int32_t get_begin_read_offset();

	/*根据偏移量获取读位置*/
	void* get_read_pos(int32_t offset_len);
	
	/*获取本次最大可读长度*/
	int32_t get_curmax_read_len(void* read_pos);

	/*移动读位置的偏移量*/
	int32_t move_read_offset(int32_t* pread_offset, int32_t offset_len);
};
