#include <mythreads.h>
#include <common_utils.h>

void ThreadCreate(Thread *thread, void *(*function)(void *), void *arg)
{
	CHECK_ERROR_ZERO(pthread_create(thread,NULL,function,arg),"Thread create");
}

void ThreadJoin(Thread thread, void **buff)
{
	CHECK_ERROR_ZERO(pthread_join(thread,NULL),"Thread join");
}

void MutexInit(Mutex *mutex)
{
    CHECK_ERROR_ZERO(pthread_mutex_init(mutex, NULL),"Mutex Init");
}

void MutexLock(Mutex *mutex)
{
    CHECK_ERROR_ZERO(pthread_mutex_lock(mutex),"Mutex Lock");
}

void MutexUnlock(Mutex *mutex)
{
    CHECK_ERROR_ZERO(pthread_mutex_unlock(mutex),"Mutex Unlock");
}

void MutexDestroy(Mutex *mutex)
{
    CHECK_ERROR_ZERO(pthread_mutex_destroy(mutex),"Mutex Destroy");
}

void MutexTrylock(Cond *cond, Mutex *mutex, const struct timespec *abstime)
{
    CHECK_ERROR_TRYLOCK(pthread_mutex_trylock(mutex), "Mutex Trylock");
}

void CondInit(Cond *cond)
{
    CHECK_ERROR_ZERO(pthread_cond_init(cond, NULL),"Cond Init");
}

void CondWait(Cond *cond,Mutex *mutex)
{
    CHECK_ERROR_ZERO(pthread_cond_wait(cond, mutex),"Cond wait");
}

void CondSignal(Cond *cond)
{
    CHECK_ERROR_ZERO(pthread_cond_signal(cond),"Cond Signal");
}

void CondDestroy(Cond *cond)
{
    CHECK_ERROR_ZERO(pthread_cond_destroy(cond),"Cond Destroy");
}

void CondBroadcast(Cond *cond)
{
    CHECK_ERROR_ZERO(pthread_cond_broadcast(cond),"Cond Broadcast");
}

void CondTimedWait(Cond *cond, Mutex *mutex, const struct timespec *abstime)
{
    CHECK_ERROR_TIMEDWAIT( pthread_cond_timedwait(cond, mutex, abstime), "Cond timed wait");
}



