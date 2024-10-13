#include "time.h"
#include <stdio.h>

int jmp_adjtimex(struct timex *buf)
{
    printf("adjtimex stub\n");
    return 0;
}

Stubs GetTimeStubs()
{
    return {
        DEF_STUB(adjtimex),
    };
}