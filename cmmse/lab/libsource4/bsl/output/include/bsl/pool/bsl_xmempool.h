/***************************************************************************
 * 
 * 
 **************************************************************************/
 
 
 
/**
 * @file bsl_xmempool.h
 * @brief 
 *  
 **/


#ifndef  __BSL_XMEMPOOL_H_
#define  __BSL_XMEMPOOL_H_

#include "bsl_pool.h"
#include <stdlib.h>
#include <stdio.h>

namespace bsl
{

union xmem_node_t
{
	xmem_node_t *next;
	char data[0];
};

class xfixmemg
{
public:
	xmem_node_t *_freelst;
	size_t _size;
public:
	inline xfixmemg() { create(0); }
	inline ~xfixmemg() { create(0); }
	inline void create (int size) {
		_size = size;
		_freelst = 0;
	}
	inline void destroy () {
		create (0);
	}
	inline void * malloc () {
		if (_freelst) {
			xmem_node_t *node = _freelst;
			_freelst = _freelst->next;
			return (void *)node->data;
		}
		return NULL;
	}

	inline void free (void *ptr) {
		((xmem_node_t *)ptr)->next = _freelst;
		_freelst = (xmem_node_t *) ptr;
	}
};

class xnofreepool : public mempool
{
public:
	char * _buffer;
	size_t _bufcap;
	size_t _bufleft;

	size_t _free;
public:
	/**
	 * @brief ����pool
	 *
	 * @param [in/out] buf   : void* �йܵ��ڴ�֧��
	 * @param [in/out] bufsiz   : size_t	�йܵ��ڴ��С
	 * @return  void 
	 * @retval   
	 * @see 
	 * @note 
	**/
	inline void create (void *buf, size_t bufsiz) {
		_buffer = (char *)buf;
		_bufcap = bufsiz;
		_bufleft = bufsiz;
		_free = 0;
	}
	inline void * malloc (size_t size) {
		if (size > _bufleft) return NULL;
		_bufleft -= size;
		return _buffer + _bufleft;
	}
	inline void free (void *, size_t size) {
		_free += size;
#ifdef BSL_NOFREEPOOL_AUTO_CLEAR
		if (_free == _bufcap) {
			clear();
		}
#endif
	}
	inline void clear () {
		_bufleft = _bufcap;
		_free = 0;
	}
	void destroy () {
		_buffer = NULL;
		_bufcap = 0;
		_bufleft = 0;
		_free = 0;
	}

};

class xmempool : public mempool
{
	static const int CNT = 10;

	xfixmemg *_pool;	//ʵ���ڴ������
	int _poolsize;
	xnofreepool _mlc; //ʵ���ڴ������
	size_t _minsiz;
	size_t _maxsiz;	//���ɷ����ڴ�
	float _rate;

public:
	xmempool ();
	~xmempool ();

	/**
	 * @brief ��ʼ���ڴ��
	 *
	 * @param [in] buf   : void* ����Ƭ�ڴ����ռ�
	 * @param [in] bufsiz   : size_t	��Ƭ�ڴ���
	 * @param [in] bmin   : size_t	��С�ڴ���䵥Ԫ���
	 * @param [in] bmax   : size_t	����ڴ���䵥Ԫ���
	 * @param [in] rate   : float	slab �ڴ�������
	 * @return  int �ɹ�����0, ����-1
	 * @retval   
	 * @see 
	 * @note 
	**/
	int create (void * buf, size_t bufsiz, 
			size_t bmin = sizeof(void *), size_t bmax = (1<<20), float rate = 2.0f);
	
	/**
	 * @brief ����pool
	 *
	 * @return  int �ɹ�����0, ʧ��-1
	 * @retval   
	 * @see 
	 * @note 
	**/
	int destroy ();


	/**
	 * @brief �����ڴ�
	 *
	 * @param [in/out] size   : size_t
	 * @return  void* ����size��С���ڴ�, ʧ�ܷ���NULL
	 * @retval   
	 * @see 
	 * @note 
	**/
	inline void * malloc (size_t size) {
		int idx = getidx (size);
		if (idx >= 0) {
			void * ret = _pool[idx].malloc();
			if (ret) return ret;
			
			return  _mlc.malloc (_pool[idx]._size);
		}
		return NULL;
	}

	/**
	 * @brief �ͷ��ڴ�
	 *
	 * @param [in/out] ptr   : void*
	 * @param [in/out] size   : size_t �ͷŵ��ڴ��С
	 * @return  void 
	 * @retval   
	 * @see 
	 * @note 
	**/
	inline void free (void *ptr, size_t size) {
		if (ptr == NULL) return;
		int idx = getidx(size);
		if (idx >= 0) {
			_pool[idx].free(ptr);
		}
	}

	/**
	 * @brief �������������ڴ�
	 *
	 * @return  size_t 
	 * @retval   
	 * @see 
	 * @note 
	**/
	inline size_t max_alloc_size() {
		return _maxsiz;
	}

	/**
	 * @brief �������з����ڴ�
	 *
	 * @return  int
	 * @retval   
	 * @see 
	 * @note 
	**/
	int clear ();


	/**
	 * @brief ��buffer�滻��ǰ�ڴ�,�����
	 *
	 * @param [in/out] buffer   : void* 
	 * @param [in/out] size   : size_t
	 * @return  int 
	 * @retval   
	 * @see 
	 * @note 
	**/
	int clear (void *buffer, size_t size);


	/**
	 * @brief ����ӿ��൱Σ��, ���Ƽ�ʹ��
	 * ����ӿڵĹ�����,�����pool�ڹ����������ڴ�,
	 * �������ڴ�Ĺ���ָ��.
	 * �����û�����clear��ʱ��,���滻�����ڴ潫������,���������·���
	 * ��������ڴ�й¶ �ļ���
	 *
	 * ����ӿڴ��ڵ�����, ���ö�ʵ�ֵ���,������������pool�����������ο���
	 *
	 * @param [in/out] buf   : void*
	 * @param [in/out] size   : size_t
	 * @return  void* 
	 * @retval   
	 * @see 
	 * @note 
	**/
	void * addbuf (void *buf, size_t size);
private:

	/**
	 * @brief ��ȡ������ڴ����ָ��
	 *
	 * @param [in/out] size   : size_t
	 * @return  int 
	 * @retval   
	 * @see 
	 * @note 
	**/
	int getidx (size_t size);

public:
	/**
	 * @brief ����size���������Ҫ�������size����ô����������Ŀռ䣬��Ҫ���ڵ���
	 *
	 * @param [in/out] size   : size_t
	 * @return  size_t 
	 * @retval   
	 * @see 
	 * @note 
	**/
	size_t goodsize (size_t size) {
		int idx =  getidx(size);
		if (idx < 0 || idx >= _poolsize) return 0;
		return _pool[idx]._size;
	}
};

};

#endif  //__BSL_XMEMPOOL_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */