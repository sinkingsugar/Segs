/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

#include <ace/Log_Msg.h>
#include "Buffer.h"

GrowingBuffer::GrowingBuffer(u8 *buf, size_t size,bool become_owner)
{
	m_buf		= NULL;
	m_size		= 0;
	m_safe_area = 0;
	m_last_err	= 0;
	m_write_off=m_read_off = 0;
	m_max_size	= size>DEFAULT_MAX_SIZE ? size:DEFAULT_MAX_SIZE;
	if(become_owner)
	{
		m_buf  = buf;
		m_size = size;
		Reset();
	}
	else
	{
		ACE_ASSERT(resize(size)==0);
		uPutBytes(buf,size);
	}
}

GrowingBuffer::GrowingBuffer(size_t max_size,u8 safe_area,size_t pre_alloc_size)
{
	m_buf = NULL;
	m_safe_area = safe_area;
	m_size = (pre_alloc_size+7)&(~7);
	m_max_size = max_size;
	m_last_err = 0;
	m_write_off=m_read_off=0;
	if(pre_alloc_size)
	{
		m_buf = new u8[m_size+m_safe_area];
        memset(m_buf,0,m_size);
		ACE_ASSERT(m_buf!=NULL);
	}
	Reset();
}
GrowingBuffer::GrowingBuffer(const GrowingBuffer &from)
{
	m_size		= from.m_size;
	m_buf		= new u8[m_size];
	ACE_ASSERT(m_buf!=NULL);
	m_last_err	= 0;
	m_write_off = from.m_write_off;
	m_safe_area = from.m_safe_area;
	m_read_off  = from.m_read_off;
	m_max_size	= from.m_max_size;
	if(m_buf&&from.m_buf)
		memcpy(m_buf,from.m_buf,m_write_off); // copy up to write point
	
}
GrowingBuffer::~GrowingBuffer()
{
	delete []m_buf;
	m_write_off=m_read_off=0;
	m_buf = 0;
}
void GrowingBuffer::PutString(const char *t)
{
	size_t len = strlen(t)+1;
	PutBytes(reinterpret_cast<const u8 *>(t),len);
}
void GrowingBuffer::uPutString(const char *t)
{
	size_t len = strlen(t);
	uPutBytes(reinterpret_cast<const u8 *>(t),len);
}

void GrowingBuffer::PutBytes(const u8 *t, size_t len)
{
	if(m_write_off+len>m_size) 
		if(resize(m_write_off+len)==-1) // space exhausted
		{
			m_last_err = 1;
			return;
		}
	uPutBytes(t,len);
}
void GrowingBuffer::uPutBytes(const u8 *t, size_t len)
{
	if(!(m_buf&&t))
		return;
	memcpy(&m_buf[m_write_off],t,len);
	m_write_off+=len;
}

void GrowingBuffer::GetString(char *t)
{
	size_t len = 0;
	if(GetReadableDataSize()==0)
	{
		m_last_err = 1;
		return;
	}
	len = strlen((char *)&m_buf[m_read_off]);
	if((0==len) || len>GetReadableDataSize())
	{
		m_last_err = 1;
		return;
	}
	uGetBytes(reinterpret_cast<u8 *>(t),len);
}
void GrowingBuffer::uGetString(char *t)
{
	size_t len(strlen((char *)&m_buf[m_read_off]));
	uGetBytes(reinterpret_cast<u8 *>(t),len);
}

bool GrowingBuffer::GetBytes(u8 *t, size_t len)
{
	if(len>GetReadableDataSize())
		return false;
	uGetBytes(t,len);
	return true;
}
void GrowingBuffer::uGetBytes(u8 *t, size_t len)
{
	memcpy(t,&m_buf[m_read_off],len);
	m_read_off += len;
}
void GrowingBuffer::PopFront(size_t pop_count)
{
	if(pop_count>m_size)
	{
		m_write_off=0;
		m_read_off=0;
		return;
	}
	if(m_write_off>=pop_count) // if there is any reason to memmove
	{
		memmove(m_buf,&m_buf[pop_count],m_write_off-pop_count); // shift buffer contents to the left
		m_write_off-=pop_count;
		if(m_read_off<pop_count)
			m_read_off=0;
		else
			m_read_off-=pop_count;

	}
	else
	{
		m_write_off=m_read_off=0;
	}
}
int GrowingBuffer::resize(size_t accommodate_size)
{
	size_t new_size = accommodate_size ? 2*accommodate_size+1 : 0;
	if(accommodate_size>m_max_size)
		return -1;
	if(accommodate_size<m_size)
		return 0;
	ACE_ASSERT(accommodate_size<0x100000);
	new_size = new_size>m_max_size ? m_max_size : new_size;
	// fix read/write indexers ( it'll happen only if new size is less then current size)
	if(m_read_off>new_size)	 m_read_off		= new_size;
	if(m_write_off>new_size) m_write_off	= new_size;

	if(0==new_size) // requested freeing of internal buffer
	{
		delete [] m_buf;
		m_buf = NULL; // this allows us to catch calls through Unchecked methods quickly
		m_size= new_size;
		return 0;
	}
	if(new_size>m_size) 
	{
		u8 *tmp = new u8[new_size+m_safe_area];
		if(NULL==tmp)
			return -2;
		ACE_ASSERT(m_write_off<=m_size); // just to be sure
		if(m_write_off>1)
			memcpy(tmp,m_buf,m_size); // copying old contents, up to actual m_write_off
        memset(&tmp[m_size],0,new_size+m_safe_area-m_size);
		delete [] m_buf;
		m_buf = tmp;
		m_size = new_size;
	}
	return 0;
}
/*
void test()
{
	static GrowingBuffer t(6);
	t.Put((u32)10);
	t.Put(1311);
	t.Put(12);
	ACE_ASSERT(t.getLastError()!=0);
	t.ResetError();
	u32 res,tres;
	t.Get(res);
	t.Get(tres);
	t.Get(res);
	ACE_ASSERT(t.getLastError()!=0);
	t.Put(res+tres);
	// unchecked tests
	t.Reset();
	t.uPut((u32)16);
	t.uPut(16);
	t.uGet(res);
	t.uPut(tres);
	t.uPut(res+tres);
}
*/
