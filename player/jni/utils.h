#ifndef __UTILS_H__
#define __UTILS_H__


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>


// for native asset manager
#include <sys/types.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>


extern "C" {

#include <pthread.h>
#include <semaphore.h>
#include <android/log.h>

} // extern "C"


//
// Android Log
//
#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "VideoChat.NDK"
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))

#ifdef  NDEBUG
#undef  LOGI
#define LOGI(...)
#endif

#ifndef UINT
typedef unsigned int UINT;
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define	FALSE 0
#endif


#define SAFE_DELETE(x) if(x) {delete x; x=NULL;}

#ifndef min
#define min(a,b) (a>b?b:a)
#endif

#ifndef max
#define max(a,b) (a>b?a:b)
#endif

//
// Thead mutex
//
class Mutex
{
public:
    Mutex()     { pthread_mutex_init( &_mutex, NULL ); }
    ~Mutex()    { pthread_mutex_destroy( &_mutex );    }

    void Lock() const   { pthread_mutex_lock( &_mutex );       }
    void Unlock() const { pthread_mutex_unlock( &_mutex );     }

    bool tryLock() const{ return 0 == pthread_mutex_trylock( &_mutex ); }
    
    inline pthread_mutex_t* mutex(){ return &_mutex; }
private:
    mutable pthread_mutex_t _mutex;
};




//
// Thead mutex AutoLock
//
class AutoLock
{
public:
    AutoLock(Mutex& mutex) : _clsMutex(mutex) { _clsMutex.Lock();   }
    ~AutoLock()                               { _clsMutex.Unlock(); }

    Mutex& _clsMutex;
};



//
// Thead Semaphore
//
class Semaphore
{
public:
    Semaphore() { sem_init(&_semaphore, 0, 0); }
    ~Semaphore() { sem_destroy(&_semaphore); }
    
    void Post() { sem_post(&_semaphore); }
    void Wait() { sem_wait(&_semaphore); }
    bool tryWait() { return 0 == sem_trywait(&_semaphore); };
    bool tryWait(uint32_t second) { timespec t = {second, 0};return 0 == sem_timedwait(&_semaphore, &t); };
    
    sem_t _semaphore;
};


template <typename T> class RingBuffer
{
private:
	T *m_buffer;
	size_t m_head;
	size_t m_tail;
	const size_t m_size;

	Mutex lock;

	size_t next(size_t current, size_t offset = 1) {
		return (current + offset) % m_size;
	}

	size_t length() { return (m_head == m_tail) ? 0 : ((m_head > m_tail) ? (m_head - m_tail) : (m_head + m_size - m_tail)); }
	size_t free()   { return (m_head == m_tail) ? m_size : ((m_head > m_tail) ? (m_tail + m_size - m_head) : (m_tail - m_head)); }

public:

	RingBuffer(const size_t size) : m_size(size), m_head(0), m_tail(0) {
		m_buffer = new T[size];
	}

	virtual ~RingBuffer() {
		delete [] m_buffer;
	}

	bool push(const T* buffer, size_t size) {
		AutoLock l(lock);
		if(free() < size)
			return false;

		size_t head = m_head;
		size_t nextHead = next(m_head, size);

		if (nextHead > head ) {
			memcpy(&m_buffer[head], buffer, size * sizeof(T));
		} else {
			memcpy(&m_buffer[head], buffer, (m_size - head) * sizeof(T));
			memcpy(&m_buffer[0], buffer + m_size - head, (size - (m_size - head)) * sizeof(T));
		}
		m_head=nextHead;

		LOGE("ringbuffer psh: %6d %6d - S:%6d", m_head, m_tail, length());

		return true;
	}

	bool push(const T &object) {
		AutoLock l(lock);
		size_t head = m_head;
		size_t nextHead = next(head);
		if (nextHead == m_tail) {
			return false;
		}
		m_buffer[head] = object;
		m_head=nextHead;

		return true;
	}

	bool pop(T* buffer, size_t size) {
		AutoLock l(lock);
		if (length() < size)
			return false;

		size_t tail = m_tail;
		size_t nextTail = next(m_tail, size);

		if (nextTail > tail) {
			memcpy(buffer, &m_buffer[tail], size * sizeof(T));
		} else {
			memcpy(buffer, &m_buffer[tail], (m_size - tail) * sizeof(T));
			memcpy(buffer + m_size - tail, &m_buffer[0], (size - (m_size - tail)) * sizeof(T));
		}
		m_tail=nextTail;

		LOGE("ringbuffer pop: %6d %6d - S:%6d", m_head, m_tail, length());

		return true;
	}

	bool pop(T &object) {
		AutoLock l(lock);
		size_t tail = m_tail;
		if (tail == m_head) {
			return false;
		}

		object = m_buffer[tail];
		m_tail=next(tail);
		return true;
	}
};


#endif //__UTILS_H__
