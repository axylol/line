#include "termios.h"
#include <errno.h>

int jmp_tcgetattr(int fd, void *a2)
{
    errno = EBADF;
    return -1;
}

Stubs GetTermiosStubs() {
    return {
        DEF_STUB(tcgetattr)
    };
}