#pragma once
#include "stdafx.h"
#define DEFAULT_BUFFER_LEN 1024*1024*3
class CCirclebuffer
{
private:
	char_t *m_header;		// buffer�Ŀ�ʼλ��

	char_t *m_tail;			// buffer�Ľ���λ��

	int32_t m_buflen;		// buffer�ĳ���

	char_t *m_curtail;		// ָ����buf��β��λ��

	int32_t m_flag_cir;		// ��־����buf�Ƿ��Ѿ�д��, ��ʼ����ʽд��
	
public:
	
	//Ĭ�Ϸ���1024*1024*3 = 3MB 
	CCirclebuffer(int32_t buf_len = DEFAULT_BUFFER_LEN );

	~CCirclebuffer(void);
	
	/*��ȡbuffer�ĳ���*/
	int32_t get_buflen();

	/*��ȡ��ʼдbuf��λ��, (��m_curtail+1 ��λ�û���m_header)*/
	void* get_write_pos();

	/*��ȡ��������д����*/
	int32_t get_curmax_write_len();

	/*�ƶ���ǰĩβ��ָ��, ����ʵ���ƶ��ĳ���*/
	int32_t move_curtail(int32_t move_len);

	/*��ȡm_header*/
	void* get_header();
	
	/*��ȡ��һ�ο�ʼ����λ�þ�m_header��ƫ����*/
	int32_t get_begin_read_offset();

	/*����ƫ������ȡ��λ��*/
	void* get_read_pos(int32_t offset_len);
	
	/*��ȡ�������ɶ�����*/
	int32_t get_curmax_read_len(void* read_pos);

	/*�ƶ���λ�õ�ƫ����*/
	int32_t move_read_offset(int32_t* pread_offset, int32_t offset_len);
};
