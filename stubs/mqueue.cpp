#include "mqueue.h"
#include <errno.h>

int jmp_mq_open()
{
    errno = ENOMEM;
    return -1;
}

int jmp_mq_unlink()
{
    errno = ENOMEM;
    return -1;
}

Stubs GetMqueueStubs() {
    return {
        DEF_STUB(mq_open),
        DEF_STUB(mq_unlink)
    };
}