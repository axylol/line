#include "libc.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "wchar.h"
#include <cctype>
#include "arith64.c"
#include <fcntl.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/mman.h>
#include "../tls.h"

int __libc_single_threaded = 0;

void jmp___libc_start_main(int (*main)(int, char **), int argc, char **ubp_av, void (*init)(void), void (*fini)(void), void (*rtld_fini)(void), void(*stack_end))
{
    char *argv[] = {(char *)"./main"};

    if (init)
        init();
    int ret = main(1, argv);
    if (fini)
        fini();

    exit(ret);
}

void jmp___gmon_start__()
{
}

void jmp__Jv_RegisterClasses()
{
}

float jmp___strtof_internal(const char *__nptr, char **__endptr, int group)
{
    return strtof(__nptr, __endptr);
}

double jmp___strtod_internal(const char *nptr, char **endptr, int group)
{
    return strtod(nptr, endptr);
}

unsigned long int jmp___strtoul_internal(const char *nptr, char **endptr, int base, int group)
{
    return strtoul(nptr, endptr, base);
}

long int jmp___strtol_internal(const char *__nptr, char **__endptr, int __base, int group)
{
    return strtol(__nptr, __endptr, __base);
}

long double jmp___strtold_internal(const char *__nptr, char **__endptr, int group)
{
    return strtold(__nptr, __endptr);
}

long jmp___wcstol_internal(const linux_wchar_t *nptr, linux_wchar_t **endptr, int base, int group)
{
    printf("__wcstol_internal stub\n");
    return 0;
}

void jmp__Unwind_RaiseException()
{
    printf("_Unwind_RaiseException stub\n");
}

void jmp__Unwind_Resume()
{
    printf("_Unwind_Resume stub\n");
}

void jmp__cxa_finalize()
{
    printf("_cxa_finalize stub\n");
}

void jmp___assert_fail()
{
    printf("__assert_fail stub called\n");
}

int32_t *to_upper_array = NULL;
int32_t *to_upper_array_offseted;

int **jmp___ctype_toupper_loc()
{
    return &to_upper_array_offseted;
}

size_t jmp___ctype_get_mb_cur_max()
{
    return __locale_mb_cur_max();
}

uint16_t *b_array = NULL;
uint16_t *b_array_offseted;

uint16_t **jmp___ctype_b_loc()
{
    return &b_array_offseted;
}

int32_t *to_lower_array = NULL;
int32_t *to_lower_array_offseted;

int32_t **jmp___ctype_tolower_loc()
{
    return &to_lower_array_offseted;
}

int *jmp___errno_location()
{
    return __errno();
}

int jmp__IO_getc(FILE *fp)
{
    // 3dx+ gets stuck while reading _IO_getc on stdin
    if (fp == stdin)
        return -1;
    return getc(fp);
}

int jmp__IO_putc(int c, FILE *fp)
{
    return putc(c, fp);
}

int jmp___sigsetjmp(sigjmp_buf env, int savemask)
{
    // printf("__sigsetjmp stub called\n");
    return 0;
    // return __extension__ ({ sigjmp_buf *_sjbuf = (sigjmp_buf*)&(env); ((*_sjbuf)[(13 * 4)] = savemask, pthread_sigmask (0, 0, (sigset_t *)((*_sjbuf) + ((13 * 4)+1))), setjmp (*_sjbuf)); });
}

int jmp___popcountsi2(int a)
{
    uint32_t x = (uint32_t)a;
    x = x - ((x >> 1) & 0x55555555);
    // Every 2 bits holds the sum of every pair of bits
    x = ((x >> 2) & 0x33333333) + (x & 0x33333333);
    // Every 4 bits holds the sum of every 4-set of bits (3 significant bits)
    x = (x + (x >> 4)) & 0x0F0F0F0F;
    // Every 8 bits holds the sum of every 8-set of bits (4 significant bits)
    x = (x + (x >> 16));
    // The lower 16 bits hold two 8 bit sums (5 significant bits).
    //    Upper 16 bits are garbage
    return (x + (x >> 8)) & 0x0000003F; // (6 significant bits)
}

uint64_t
jmp___fixunsdfdi(double a)
{
    if (a <= 0.0)
        return 0;
    uint32_t high = a / 0x1p32f;
    uint32_t low = a - (double)high * 0x1p32f;
    return ((uint64_t)high << 32) | low;
}

long double jmp___powixf2(long double a, int b)
{
    const int recip = b < 0;
    long double r = 1;
    while (1)
    {
        if (b & 1)
            r *= a;
        b /= 2;
        if (b == 0)
            break;
        a *= a;
    }
    return recip ? 1 / r : r;
}

char *jmp___strtok_r(char *s, const char *delim, char **save_ptr)
{
    return strtok_r(s, delim, save_ptr);
}

int jmp_system(const char *string)
{
    printf("system(%s)\n", string);
    return 0;
}

int jmp_setenv(const char *name, const char *value, int overwrite)
{
    printf("setenv(\"%s\", \"%s\")\n", name, value);
    return setenv(name, value, overwrite);
}

char *jmp_getenv(const char *name)
{
    printf("getenv(\"%s\")\n", name);
    return getenv(name);
}

int jmp_eventfd(unsigned int initval, int flags)
{
    errno = ENOMEM;
    return -1;
}

int jmp_mlockall(int flags)
{
    return 0;
}
int jmp_munlockall()
{
    return 0;
}

FILE *jmp_popen(const char *command, const char *type)
{
    printf("popen(%s, %s)\n", command, type);
    return NULL;
}

int jmp___fprintf_chk(FILE *stream, int flag, const char *format, ...)
{
    va_list arg;
    va_start(arg, format);
    int length = vfprintf(stream, format, arg);
    va_end(arg);
    return length;
}

int jmp___printf_chk(int flag, const char *format, ...)
{
    va_list arg;
    va_start(arg, format);
    int length = vprintf(format, arg);
    va_end(arg);
    return length;
}

// im not implementing this just for isoc99 :skull:
int jmp___isoc99_sscanf(char *str, const char *format, ...)
{
    va_list arg;
    va_start(arg, format);
    int length = vsscanf(str, format, arg);
    va_end(arg);
    return length;
}

_sig_func_ptr jmp___sysv_signal(int signum, _sig_func_ptr handler)
{
    printf("__sysv_signal stub called\n");
    return SIG_ERR;
}

struct tls_index
{
    unsigned long int ti_module;
    unsigned long int ti_offset;
};

void *jmp____tls_get_addr(tls_index *ti)
{
    printf("___tls_get_addr stub called\n");
    return TlsGetAddr(ti->ti_module, ti->ti_offset);
}

int jmp_syscall(int code, ...)
{
    printf("syscall %d stub called\n", code);

    errno = ENOSYS;
    return -1;
}

void jmp___longjmp_chk(jmp_buf env, int val)
{
    printf("__longjmp_chk stub called\n");
    // longjmp(env, val);
}

void *jmp_mmap64(void *addr, size_t length, int prot, int flags, int fd, int64_t offset)
{
    return mmap(addr, length, prot, flags, fd, offset);
}

Stubs GetLibcStubs()
{
    static bool initialized = false;
    if (!initialized)
    {
        initialized = true;

        to_lower_array = (int32_t *)malloc(384 * sizeof(int32_t));
        for (int i = 0; i < 384; i++)
        {
            int c = i - 128;
            to_lower_array[i] = std::tolower(c);
        }
        to_lower_array_offseted = &to_lower_array[128];

        to_upper_array = (int32_t *)malloc(384 * sizeof(int32_t));
        for (int i = 0; i < 384; i++)
        {
            int c = i - 128;
            to_upper_array[i] = std::toupper(c);
        }
        to_upper_array_offseted = &to_upper_array[128];

        b_array = (uint16_t *)malloc(384 * sizeof(uint16_t));
        for (int i = 0; i < 384; i++)
        {
            uint16_t flags = 0;
#define _ISbit(bit) ((bit) < 8 ? ((1 << (bit)) << 8) : ((1 << (bit)) >> 8))

            int c = i - 128;
            if (std::isupper(c))
                flags |= _ISbit(0);
            if (std::islower(c))
                flags |= _ISbit(1);
            if (std::isalpha(c))
                flags |= _ISbit(2);
            if (std::isdigit(c))
                flags |= _ISbit(3);
            if (std::isxdigit(c))
                flags |= _ISbit(4);
            if (std::isspace(c))
                flags |= _ISbit(5);
            if (std::isprint(c))
                flags |= _ISbit(6);
            if (std::isgraph(c))
                flags |= _ISbit(7);
            if (std::isblank(c))
                flags |= _ISbit(8);
            if (std::iscntrl(c))
                flags |= _ISbit(9);
            if (std::ispunct(c))
                flags |= _ISbit(10);
            if (std::isalnum(c))
                flags |= _ISbit(11);

            b_array[i] = flags;
        }

        b_array_offseted = &b_array[128];
    }

    return {

        {"stdout", &stdout},
        {"stderr", &stderr},
        {"stdin", &stdin},

        {"__libc_single_threaded", &__libc_single_threaded},

        DEF_STUB(__libc_start_main),
        DEF_STUB(__gmon_start__),
        DEF_STUB(_Jv_RegisterClasses),

        DEF_STUB(__strtof_internal),
        DEF_STUB(__strtod_internal),
        DEF_STUB(__strtoul_internal),
        DEF_STUB(__strtol_internal),
        DEF_STUB(__strtold_internal),

        DEF_STUB(__wcstol_internal),

        DEF_STUB(_Unwind_RaiseException),
        DEF_STUB(_Unwind_Resume),
        DEF_STUB(_cxa_finalize),
        DEF_STUB(__assert_fail),

        DEF_STUB(__ctype_tolower_loc),
        DEF_STUB(__ctype_toupper_loc),
        DEF_STUB(__ctype_get_mb_cur_max),
        DEF_STUB(__ctype_b_loc),

        DEF_STUB(__errno_location),

        DEF_STUB(_IO_getc),
        DEF_STUB(_IO_putc),

        DEF_STUB(__sigsetjmp),

        {"__umoddi3", (void *)__umoddi3},
        {"__divdi3", (void *)__divdi3},
        {"__moddi3", (void *)__moddi3},
        {"__udivdi3", (void *)__udivdi3},
        {"__divmoddi4", (void *)__divmoddi4},

        DEF_STUB(__popcountsi2),
        DEF_STUB(__fixunsdfdi),
        DEF_STUB(__powixf2),
        DEF_STUB(__strtok_r),

        DEF_STUB(system),

        DEF_STUB(getenv),
        DEF_STUB(setenv),

        DEF_STUB(eventfd),

        DEF_STUB(mlockall),
        DEF_STUB(munlockall),

        DEF_STUB(popen),

        DEF_STUB(__fprintf_chk),
        DEF_STUB(__printf_chk),
        DEF_STUB(__isoc99_sscanf),

        DEF_STUB(__sysv_signal),

        DEF_STUB(___tls_get_addr),
        DEF_STUB(syscall),

        DEF_STUB(__longjmp_chk),

        DEF_STUB(mmap64),
    };
}