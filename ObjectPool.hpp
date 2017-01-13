#ifndef __OBJECT_POOL_HPP__
#define __OBJECT_POOL_HPP__

#include <Windows.h>
#include <list>
/**
* Description:	Define platform portable types.
*/
class VehiclePort
{
	/**
	* Description:	Define native types.
	*/
protected:
	/**
	* Description:	Define locker.
	*/
	class Locker
	{
		/**
		* Description:	Define construct interfaces.
		*/
	public:
		Locker() { InitializeCriticalSection(&m_cs); }
		~Locker() { DeleteCriticalSection(&m_cs); }

		/**
		* Description:	Define public interfaces.
		*/
	public:
		void Lock(void) { EnterCriticalSection(&m_cs); }
		void Unlock(void) { LeaveCriticalSection(&m_cs); }

		/**
		* Description:	Define inner variables.
		*/
	private:
		CRITICAL_SECTION m_cs;
	};

	/**
	* Description:	Define auto locker.
	*/
	class AutoLocker
	{
		/**
		* Description:	Define construct interfaces.
		*/
	public:
		AutoLocker(Locker &locker) : m_locker(locker) { m_locker.Lock(); }
		~AutoLocker() { m_locker.Unlock(); }

		/**
		* Description:	Define inner variables.
		*/
	private:
		Locker &m_locker;
	};

	/**
	* Description:	Define semaphore.
	*/
	class Semaphore
	{
		/**
		* Description:	Define construct interfaces.
		*/
	public:
		Semaphore() { m_handle = CreateSemaphore(NULL, 0, 1 << 30, NULL); }
		~Semaphore() { CloseHandle(m_handle); }

		/**
		* Description:	Define public interfaces.
		*/
	public:
		bool Wait(DWORD timeout) { return WAIT_OBJECT_0 == WaitForSingleObject(m_handle, timeout); }
		void Post(int count = 1)
		{
			while (1)
			{
				BOOL ok = ReleaseSemaphore(m_handle, count, NULL);
				if (!ok && ERROR_TOO_MANY_POSTS == GetLastError())
				{
					Sleep(1);
					continue;
				}
				break;
			}
		}

		/**
		* Description:	Define innser variables.
		*/
	private:
		HANDLE m_handle;
	};

	/**
	* Description:	Define unsigned long interger.
	*/
	typedef unsigned long long UINT64;
};


/**
* Description:	Define object pool.
*/
template <typename ObjType, int BUNCH_SIZE = 4096, int POOL_SIZE = -1>
class ObjectPool : public VehiclePort
{
	/**
	* Description:	Define construct interfaces.
	*/
public:
	ObjectPool() : m_counter(0) {}
	~ObjectPool()
	{
		/**
		* Description:	Destroy all objects allocated.
		*/
		for (ObjMap::iterator i = m_busy.begin(); i != m_busy.end(); ++i)
		{
			ObjType *obj = i->first;

			obj->~ObjType();
			m_free.insert(std::make_pair(i->second, obj));
		}

		/**
		* Description:	Free all bunch memory.
		*/
		while (m_bunch.size() > 0)
		{
			void *base = m_bunch.front();

			m_bunch.pop_front();
			free(base);
		}
	}


	/**
	* Description:	Define public interfaces.
	*/
public:
	/**
	* Function:	New().
	* Input:		None.
	* Output:		None.
	* Return:		Object pointer.
	* Description:	Create a object.
	*/
	ObjType *New(void)
	{
		AutoLocker lock(m_locker);

		//printf("m_free:%d   m_busy:%d  %s\n", m_free.size(), m_busy.size(), typeid(*this).name());

		/**
		* Description:	No free memory?
		*/
		if (m_free.size() == 0)
		{
			/**
			* Description:	Check pool size.
			*/
			if ((POOL_SIZE > 0) && (m_busy.size() > POOL_SIZE))
			{
				printf("%s() too much object in pool(%d, %d)!\n", __FUNCTION__, m_busy.size(), m_free.size());
				return NULL;
			}

			/**
			* Description:	Create new bunch.
			*/
			ObjType *bunch = (ObjType *)calloc(1, BUNCH_SIZE);
			if (NULL == bunch)
			{
				printf("%s() allocate object bunch failed(%)!\n", __FUNCTION__, BUNCH_SIZE);
				return NULL;
			}

			/**
			* Description:	Record bunch and splite bunch into objects.
			*/
			m_bunch.push_back(bunch);
			for (int i = 0; i<BUNCH_SIZE / sizeof(ObjType); ++i)
			{
				m_free.insert(std::make_pair(InterlockedIncrement(&m_counter), &bunch[i]));
			}
		}

		/**
		* Description:	Get object from free pool.
		*/
		ObjType *obj = m_free.begin()->second;

		/**
		* Description:	Create object.
		*/
		//try
		//{
		//	::new(obj) ObjType();
		//}
		//catch (...)
		//{
		//	return NULL;
		//}

		/**
		* Description:	Record object in busy set.
		*/
		if (!m_busy.insert(std::make_pair(obj, m_free.begin()->first)).second)
		{
			printf("%s() insert object(%p) into busy set failed!\n", __FUNCTION__, obj);
			//NPLogError(("%s() insert object(%p) into busy set failed!\n", __FUNCTION__, obj));
		}

		m_free.erase(m_free.begin());

		return obj;
	}


	/**
	* Function:	Free().
	* Input:		obj:	Object pointer.
	* Output:		None.
	* Return:		None.
	* Description:	Destroy a object.
	*/
	void Free(ObjType *&obj)
	{
		AutoLocker lock(m_locker);

		/**
		* Description:	Check pointer by busy set.
		*/
		ObjMap::iterator i = m_busy.find(obj);
		if (i == m_busy.end())
		{
			//NPLogError(("%s() object(%p) is not in record!\n", __FUNCTION__, obj));
			printf("%s() object(%p) is not in record!\n", __FUNCTION__, obj);
			obj = NULL;
			return;
		}

		/**
		* Description:	Destroy object, move it from busy set to free list.
		*/
		obj->~ObjType();
		m_free.insert(std::make_pair(i->second, obj));
		m_busy.erase(i);
		obj = NULL;
	}


	/**
	* Description:	Define inner types.
	*/
private:
	typedef std::list<void *>		  BunchList;
	typedef std::map<long, ObjType *> ObjList;		/* Use set to reuse object recently.						*/
	typedef std::map<ObjType *, long> ObjMap;


	/**
	* Description:	Define inner variables.
	*/
private:
	Locker		  m_locker;
	volatile long m_counter;
	ObjList		  m_free;
	ObjMap		  m_busy;
	BunchList	  m_bunch;
};


typedef ObjectPool<MemObj, 128 * 1024> MemPool;

#endif