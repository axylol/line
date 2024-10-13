#include "ioctl.h"

#include <sys/ioctl.h>
#include <stdio.h>

int jmp_ioctl(int fd, unsigned long op, int *a3)
{
    //printf("ioctl %d %d\n", fd, op);
    switch (op)
    {
    case 0x5415: // TIOCMGET
        return -1;
    case 0x5421: // FIONBIO
    {
        op = FIONBIO;
        break;
    }
    case 0x8913: // SIOCGIFFLAGS
        return -1;
    case 0x8915: // SIOCGIFADDR
        return -1;
    case 0x8927: // SIOCGIFHWADDR
        return 0;
    case 0x541e: // TIOCGSERIAL
    case 0x541f:  // TIOCSSERIAL
        return -1;
    default:
    {
        printf("Untranslated ioctl %d\n", op);
        break;
    }
    }

    return ioctl(fd, op, a3);
}

Stubs GetIoctlStubs()
{
    return {
        DEF_STUB(ioctl)};
}