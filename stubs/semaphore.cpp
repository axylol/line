#include "semaphore.h"
#include <semaphore.h>
#include <string.h>
#include <errno.h>

int jmp_sem_init(sem_t *sem, int pshared, unsigned int value)
{
    memset(sem, 0, 16);
    return sem_init(sem, pshared, value);
}

int jmp_sem_timedwait(sem_t *sem, const struct timespec *abs_timeout)
{
    int ret = sem_timedwait(sem, abs_timeout);
    if (ret == ETIMEDOUT)
        return 110;
    return ret;
}

Stubs GetSemaphoreStubs()
{
    return {
        DEF_STUB(sem_init),
        DEF_STUB(sem_timedwait)};
}