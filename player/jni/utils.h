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



#endif //__UTILS_H__