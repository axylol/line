#include "pthread.h"
#include <string.h>
#include <pthread.h>
#include <limits.h>
#include <iostream>
#include "../config.h"

int jmp_pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
    *(int *)mutex = 0;
    return pthread_mutex_init(mutex, attr);
}

int jmp_pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr)
{
    *(int *)cond = 0;
    return pthread_cond_init((pthread_cond_t *)cond, attr);
}

int jmp_pthread_attr_init(pthread_attr_t *attr)
{
    *(int *)attr = 0;
    return pthread_attr_init(attr);
}

int jmp_pthread_spin_init(pthread_spinlock_t *lock, int pshared)
{
    *(int *)lock = 0;
    return pthread_spin_init(lock, pshared);
}

int jmp_pthread_once(pthread_once_t *once_control, void (*init_routine)(void))
{
    pthread_once_t wrap;
    wrap.mutex = 0;
    wrap.state = *(int *)once_control;

    int ret = pthread_once(&wrap, init_routine);

    *(int *)once_control = wrap.state;

    return ret;
}

#ifdef MUTEX_CHECK
#define MUTEX_ENSURE_INITIALIZED(mutex) \
    if (*(int *)mutex == 0)             \
    {                                   \
        pthread_mutex_init(mutex, 0);   \
    }
#define COND_ENSURE_INITIALIZED(cond) \
    if (*(int *)cond == 0)            \
    {                                 \
        pthread_cond_init(cond, 0);   \
    }
#define SPIN_ENSURE_INITIALIZED(spin) \
    if (*(int *)spin == 0)            \
    {                                 \
        pthread_spin_init(spin, 0);   \
    }
#else
#define MUTEX_ENSURE_INITIALIZED(mutex)
#define COND_ENSURE_INITIALIZED(cond)
#define SPIN_ENSURE_INITIALIZED(spin)
#endif

int jmp_pthread_mutex_lock(pthread_mutex_t *mutex)
{
    MUTEX_ENSURE_INITIALIZED(mutex);

    int result = pthread_mutex_lock(mutex);
#ifdef MUTEX_CHECK
    if (result != EINVAL)
        return result;
    *(int *)mutex = 0;
    pthread_mutex_init(mutex, 0);
    return pthread_mutex_lock(mutex);
#else
    return result;
#endif
}

int jmp_pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    MUTEX_ENSURE_INITIALIZED(mutex);
    int result = pthread_mutex_unlock(mutex);
#ifdef MUTEX_CHECK
    if (result != EINVAL)
        return result;
    *(int *)mutex = 0;
    pthread_mutex_init(mutex, 0);
    return pthread_mutex_unlock(mutex);
#else
    return result;
#endif
}

int jmp_pthread_mutex_trylock(pthread_mutex_t *mutex)
{
    MUTEX_ENSURE_INITIALIZED(mutex);
    int result = pthread_mutex_trylock(mutex);
#ifdef MUTEX_CHECK
    if (result != EINVAL)
        return result;
    *(int *)mutex = 0;
    pthread_mutex_init(mutex, 0);
    return pthread_mutex_trylock(mutex);
#else
    return result;
#endif
}

int jmp_pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
    COND_ENSURE_INITIALIZED(cond);
    MUTEX_ENSURE_INITIALIZED(mutex);

    int ret = pthread_cond_wait(cond, mutex);
#ifdef MUTEX_CHECK
    if (ret != EINVAL)
        return ret;
    ret = pthread_mutex_trylock(mutex);
    if (ret == 0)
    {
        pthread_mutex_unlock(mutex);
    }
    else if (ret == EINVAL)
    {
        pthread_mutex_init(mutex, 0);
    }

    ret = pthread_cond_wait(cond, mutex);
    if (ret != EINVAL)
        return ret;
    *(int *)cond = 0;
    pthread_cond_init(cond, 0);
    return pthread_cond_wait(cond, mutex);
#else
    return ret;
#endif
}

int jmp_pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime)
{
    COND_ENSURE_INITIALIZED(cond);
    MUTEX_ENSURE_INITIALIZED(mutex);

    int ret = pthread_cond_timedwait(cond, mutex, abstime);
    if (ret == ETIMEDOUT)
        return 110;
#ifdef MUTEX_CHECK
    if (ret != EINVAL)
        return ret;
    ret = pthread_mutex_trylock(mutex);
    if (ret == 0)
    {
        pthread_mutex_unlock(mutex);
    }
    else if (ret == EINVAL)
    {
        pthread_mutex_init(mutex, 0);
    }

    ret = pthread_cond_timedwait(cond, mutex, abstime);
    if (ret != EINVAL)
        return ret;
    *(int *)cond = 0;
    pthread_cond_init(cond, 0);
    ret = pthread_cond_timedwait(cond, mutex, abstime);
    if (ret == ETIMEDOUT)
        return 110;
#endif
    return ret;
}

int jmp_pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type)
{
    switch (type)
    {
    case 0:
    {
        type = PTHREAD_MUTEX_NORMAL;
        break;
    }
    case 1:
    {
        type = PTHREAD_MUTEX_RECURSIVE;
        break;
    }
    case 2:
    {
        type = PTHREAD_MUTEX_ERRORCHECK;
        break;
    }
    case 3:
    {
        type = PTHREAD_MUTEX_DEFAULT;
        break;
    }
    }
    return pthread_mutexattr_settype(attr, type);
}

int jmp_pthread_sigmask(int how, const sigset_t *set, sigset_t *oldset)
{
    switch (how)
    {
    case 0:
    {
        how = SIG_BLOCK;
        break;
    }
    case 1:
    {
        how = SIG_UNBLOCK;
        break;
    }
    case 2:
    {
        how = SIG_SETMASK;
        break;
    }
    }
    return pthread_sigmask(how, set, oldset);
}

int jmp_pthread_cond_broadcast(pthread_cond_t *cond)
{
    COND_ENSURE_INITIALIZED(cond);
    int ret = pthread_cond_broadcast(cond);
#ifdef MUTEX_CHECK
    if (ret != EINVAL)
        return ret;
    *(int *)cond = 0;
    pthread_cond_init(cond, 0);
    return pthread_cond_broadcast(cond);
#else
    return ret;
#endif
}

int jmp_pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize)
{
    if (stacksize < PTHREAD_STACK_MIN)
        stacksize = PTHREAD_STACK_MIN;
    return pthread_attr_setstacksize(attr, stacksize);
}

int jmp_pthread_attr_setstack(pthread_attr_t *attr, void* stackaddr, size_t stacksize)
{
    if (stacksize < PTHREAD_STACK_MIN)
        printf("pthread_attr_setstack SIZE TOO LOW\n");
    memset(stackaddr, 0, stacksize);
    return pthread_attr_setstack(attr, stackaddr, stacksize);
}

int jmp_pthread_attr_setschedparam(pthread_attr_t *attr,
                                   const struct sched_param *param)
{
    int ret = pthread_attr_setschedparam(attr, param);
    if (ret == ENOTSUP)
        return 0;
    return ret;
}

void jmp___pthread_unregister_cancel(void *buf)
{
}

int jmp_pthread_attr_setaffinity_np(pthread_attr_t *attr,
                                    size_t cpusetsize, const void *cpuset)
{
    return 0;
}

void jmp___pthread_register_cancel(void *buf)
{
}

int jmp_pthread_cond_signal(pthread_cond_t *cond)
{
    COND_ENSURE_INITIALIZED(cond);

    int ret = pthread_cond_signal(cond);
#ifdef MUTEX_CHECK
    if (ret != EINVAL)
        return ret;
    *(int *)cond = 0;
    pthread_cond_init(cond, 0);
    return pthread_cond_signal(cond);
#else
    return ret;
#endif
}

int jmp_pthread_spin_lock(pthread_spinlock_t *lock)
{
    SPIN_ENSURE_INITIALIZED(lock);

    int ret = pthread_spin_lock(lock);
#ifdef MUTEX_CHECK
    if (ret != EINVAL)
        return ret;
    *(int *)lock = 0;
    pthread_spin_init(lock, 0);
    return pthread_spin_lock(lock);
#else
    return ret;
#endif
}
int jmp_pthread_spin_trylock(pthread_spinlock_t *lock)
{
    SPIN_ENSURE_INITIALIZED(lock);

    int ret = pthread_spin_trylock(lock);
#ifdef MUTEX_CHECK
    if (ret != EINVAL)
        return ret;
    *(int *)lock = 0;
    pthread_spin_init(lock, 0);
    return pthread_spin_trylock(lock);
#else
    return ret;
#endif
}
int jmp_pthread_spin_unlock(pthread_spinlock_t *lock)
{
    SPIN_ENSURE_INITIALIZED(lock);

    int ret = pthread_spin_unlock(lock);
#ifdef MUTEX_CHECK
    if (ret != EINVAL)
        return ret;
    *(int *)lock = 0;
    pthread_spin_init(lock, 0);
    return pthread_spin_unlock(lock);
#else
    return ret;
#endif
}

int jmp_pthread_mutex_timedlock(pthread_mutex_t *mutex, const struct timespec *abs_timeout)
{
    MUTEX_ENSURE_INITIALIZED(mutex);

    int ret = pthread_mutex_timedlock(mutex, abs_timeout);
    if (ret == ETIMEDOUT)
        return 110;
#ifdef MUTEX_CHECK
    if (ret != EINVAL)
        return ret;
    *(int *)mutex = 0;
    pthread_mutex_init(mutex, 0);
    ret = pthread_mutex_timedlock(mutex, abs_timeout);
    if (ret == ETIMEDOUT)
        return 110;
#endif
    return ret;
}

void jmp___pthread_unwind_next(void *buf)
{
    printf("__pthread_unwind_next stub called\n");
}


Stubs GetPthreadStubs()
{
    return {
        DEF_STUB(pthread_mutex_init),
        DEF_STUB(pthread_cond_init),
        DEF_STUB(pthread_attr_init),
        DEF_STUB(pthread_spin_init),

        DEF_STUB(pthread_once),
        DEF_STUB(pthread_mutex_lock),
        DEF_STUB(pthread_mutex_unlock),
        DEF_STUB(pthread_mutex_trylock),
        DEF_STUB(pthread_cond_wait),
        DEF_STUB(pthread_cond_timedwait),
        DEF_STUB(pthread_mutexattr_settype),
        DEF_STUB(pthread_sigmask),

        DEF_STUB(pthread_cond_broadcast),
        DEF_STUB(pthread_attr_setstacksize),
        DEF_STUB(pthread_attr_setstack),
        DEF_STUB(pthread_attr_setschedparam),

        DEF_STUB(__pthread_unregister_cancel),
        DEF_STUB(pthread_attr_setaffinity_np),
        DEF_STUB(__pthread_register_cancel),

        DEF_STUB(pthread_cond_signal),

        DEF_STUB(pthread_spin_lock),
        DEF_STUB(pthread_spin_trylock),
        DEF_STUB(pthread_spin_unlock),

        DEF_STUB(pthread_mutex_timedlock),

        DEF_STUB(__pthread_unwind_next),

    };
}