#include "wchar.h"
#include <stdlib.h>

size_t jmp_wcslen(const linux_wchar_t *s)
{
    size_t i = 0;
    while (true)
    {
        if (!s[i])
            break;
        i++;
    }
    return i;
}

linux_wchar_t *jmp_wmemcpy(linux_wchar_t *dest, const linux_wchar_t *src, size_t count)
{
    for (size_t i = 0; i < count; i++)
        dest[i] = src[i];
    return dest;
}

int jmp_wcscmp(const linux_wchar_t *s1, const linux_wchar_t *s2)
{
    size_t i = 0;
    while (s2[i])
    {
        if (s1[i] != s2[i])
            return s1[i] > s2[i] ? 1 : -1;
        i++;
    }
    return 0;
}

linux_wchar_t *jmp_wcscpy(linux_wchar_t *s1, const linux_wchar_t *s2)
{
    size_t i = 0;
    while (true)
    {
        s1[i] = s2[i];
        if (!s2[i])
            break;
        i++;
    }
    return s1;
}

int jmp_wmemcmp(const linux_wchar_t *s1, const linux_wchar_t *s2, size_t n)
{
    for (size_t i = 0; i < n; i++)
        if (s1[i] != s2[i])
            return s1[i] > s2[i] ? 1 : -1;
    return 0;
}

linux_wchar_t *jmp_wmemset(linux_wchar_t *ptr, linux_wchar_t c, size_t num)
{
    for (size_t i = 0; i < num; i++)
        ptr[i] = c;
    return ptr;
}

linux_wchar_t *jmp_wmemchr(linux_wchar_t *s, linux_wchar_t c, size_t n)
{
    for (size_t i = 0; i < n; i++)
        if (s[i] == c)
            return &s[i];
    return NULL;
}

linux_wchar_t *jmp_wmemmove(linux_wchar_t *d, const linux_wchar_t *s, size_t n)
{
    auto temp = (linux_wchar_t *)malloc(sizeof(linux_wchar_t) * n);
    if (!temp)
        return NULL;
    for (size_t i = 0; i < n; i++)
        temp[i] = s[i];
    for (size_t i = 0; i < n; i++)
        d[i] = temp[i];
    free(temp);
    return d;
}

Stubs GetWcharStubs()
{
    return {
        // Windows wchar_t is 2 bytes and linux is 4 bytes, so we should implement
        // our own function that uses 4 byte wchar_t (int) instead
        DEF_STUB(wcslen),
        DEF_STUB(wmemcpy),
        DEF_STUB(wcscmp),
        DEF_STUB(wcscpy),
        DEF_STUB(wmemcmp),
        DEF_STUB(wmemset),
        DEF_STUB(wmemchr),
        DEF_STUB(wmemmove),
    };
}