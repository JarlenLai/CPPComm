#pragma once

#include <memory.h>
#include <malloc.h>

#define MyMax(a, b) ((a)>(b) ? (a):(b))
#define MyMin(a, b) ((a)<(b) ? (a):(b))

template<class T, size_t MAX_CACHE_SIZE = 0>
class CBufferPtrT
{
public:
	explicit CBufferPtrT(size_t size = 0, bool zero = false)		{Reset(); Malloc(size, zero);}
	explicit CBufferPtrT(const T* pch, size_t size)					{Reset(); Copy(pch, size);}
	CBufferPtrT(const CBufferPtrT& other)							{Reset(); Copy(other);}
	template<size_t S> CBufferPtrT(const CBufferPtrT<T, S>& other)  {Reset(); Copy(other);}
	CBufferPtrT(int nLen, const char* format, ...) 
	{
		char* pBuff = new char[nLen + 1]();//默认使用char的构造函数初始化为0了
		va_list arg_ptr;
		va_start(arg_ptr, format);
		_vsnprintf_s(pBuff, nLen, _TRUNCATE, format, arg_ptr);
		va_end(arg_ptr);

		Reset();
		Copy(pBuff, strlen(pBuff)+1);
		delete[] pBuff;
	}

	~CBufferPtrT() {Free();}

	T* Malloc(size_t size = 1, bool zero = false)
	{
		Free();
		return Alloc(size, zero, false);
	}

	T* Realloc(size_t size, bool zero = false)
	{
		return Alloc(size, zero, true);
	}

	void Free()
	{
		if(m_pch)
		{
			free(m_pch);
			Reset();
		}
	}

	template<size_t S> CBufferPtrT& Copy(const CBufferPtrT<T, S>& other)
	{
		if((void*)&other != (void*)this)
			Copy(other.Ptr(), other.Size());

		return *this;
	}

	CBufferPtrT& Copy(const T* pch, size_t size)
	{
		Malloc(size);

		if(m_pch)
			memcpy(m_pch, pch, size * sizeof(T));

		return *this;
	}

	CBufferPtrT& MemMoveUse(size_t use)
	{
		if (use >= m_size && m_pch)
		{
			memset(m_pch, 0, m_size * sizeof(T));
			SetSize(0);
		}
		else if (m_pch && use > 0)
		{
			memmove(m_pch, m_pch + use, (m_size - use) * sizeof(T));
			SetSize(m_size - use);
		}

		return *this;
	}

	template<size_t S> CBufferPtrT& Cat(const CBufferPtrT<T, S>& other)
	{
		if((void*)&other != (void*)this)
			Cat(other.Ptr(), other.Size());

		return *this;
	}

	CBufferPtrT& Cat(const T* pch, size_t size = 1)
	{
		size_t pre_size = m_size;
		Realloc(m_size + size);

		if(m_pch)
			memcpy(m_pch + pre_size, pch, size * sizeof(T));

		return *this;
	}

	template<size_t S> bool Equal(const CBufferPtrT<T, S>& other) const
	{
		if((void*)&other == (void*)this)
			return true;
		else if(m_size != other.Size())
			return false;
		else if(m_size == 0)
			return true;
		else
			return (memcmp(m_pch, other.Ptr(), m_size * sizeof(T)) == 0);
	}

	bool Equal(T* pch) const
	{
		if(m_pch == pch)
			return true;
		else if(!m_pch || !pch)
			return false;
		else
			return (memcmp(m_pch, pch, m_size * sizeof(T)) == 0);
	}

	size_t SetSize(size_t size)
	{
		if(size < 0 || size > m_capacity)
			size = m_capacity;

		return (m_size = size);
	}

	T*			Ptr()					{return m_pch;}
	const T*	Ptr()			const	{return m_pch;}
	T&			Get(int i)				{return *(m_pch + i);}
	const T&	Get(int i)		const	{return *(m_pch + i);}
	size_t		Size()			const	{return m_size;}
	size_t		Capacity()		const	{return m_capacity;}
	bool		IsValid()		const	{return m_pch != 0;}

	operator bool()                                                             {return IsValid();}
	operator							T*	()									{return Ptr();}
	operator const						T*	()			const					{return Ptr();}
	T& operator							[]	(int i)								{return Get(i);}
	const T& operator					[]	(int i)		const					{return Get(i);}
	bool operator						==	(T* pv)		const					{return Equal(pv);}
	template<size_t S> bool operator	==	(const CBufferPtrT<T, S>& other)	{return Equal(other);}
	CBufferPtrT& operator				=	(const CBufferPtrT& other)			{return Copy(other);}
	template<size_t S> CBufferPtrT& operator = (const CBufferPtrT<T, S>& other)	{return Copy(other);}

private:
	void Reset()						{m_pch = 0; m_size = 0; m_capacity = 0;}
	size_t GetAllocSize(size_t size)	{return MyMax(size, MyMin(size * 2, m_size + MAX_CACHE_SIZE));}

	T* Alloc(size_t size, bool zero = false, bool is_realloc = false)
	{
		if(size != m_size)
		{
			size_t rsize = GetAllocSize(size);
			if(size > m_capacity || rsize < m_size)
			{
				T* pch = is_realloc							?
					(T*)realloc(m_pch, rsize * sizeof(T))	:
					(T*)malloc(rsize * sizeof(T))			;

				if(pch || rsize == 0)
				{
					m_pch		= pch;
					m_size		= size;
					m_capacity	= rsize;
				}
				else
				{
					Free();
					throw std::bad_alloc();
				}
			}
			else
				m_size = size;
		}

		if(zero && m_pch)
			memset(m_pch, 0, m_size * sizeof(T));

		return m_pch;
	}

private:
	T*		m_pch;
	size_t	m_size;
	size_t	m_capacity;
};

typedef CBufferPtrT<char>			CCharBufferPtr;
typedef CBufferPtrT<wchar_t>		CWCharBufferPtr;
typedef CBufferPtrT<unsigned char>	CByteBufferPtr;
typedef CByteBufferPtr				CBufferPtr;

#ifdef _UNICODE
	typedef CWCharBufferPtr			CTCharBufferPtr;
#else
	typedef CCharBufferPtr			CTCharBufferPtr;
#endif
