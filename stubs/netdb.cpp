#include "netdb.h"
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include "../config.h"

#pragma pack(push, 1)
struct linux_addrinfo
{
    int ai_flags;
    int ai_family;
    int ai_socktype;
    int ai_protocol;
    socklen_t ai_addrlen;
    struct sockaddr *ai_addr;
    char *ai_canonname;
    struct addrinfo *ai_next;
};

struct linux_hostent
{
    char *h_name;       /* official name of host */
    char **h_aliases;   /* alias list */
    int h_addrtype;     /* host address type */
    int h_length;       /* length of address */
    char **h_addr_list; /* list of addresses */
};

struct linux_servent
{
    char *s_name;     /* official service name */
    char **s_aliases; /* alias list */
    int s_port;       /* port number */
    char *s_proto;    /* protocol to use */
};

#pragma pack(pop)

static_assert(sizeof(linux_addrinfo) == 32, "size mismatch");
static_assert(sizeof(struct linux_hostent) == 20, "size mismatch");
static_assert(sizeof(struct linux_servent) == 16, "size mismatch");

void translate_addrinfo_to_msys(struct addrinfo *addr)
{
    // backup
    struct sockaddr *ai_addr = addr->ai_addr;

    // reverse
    addr->ai_addr = (struct sockaddr *)addr->ai_canonname;
    addr->ai_canonname = (char *)ai_addr;

    if (addr->ai_next)
        translate_addrinfo_to_msys(addr->ai_next);
}

int jmp_getaddrinfo(const char *node, const char *service, const struct linux_addrinfo *hints, struct linux_addrinfo **res)
{
    /*if (hints)
        translate_addrinfo_to_msys((struct addrinfo *)hints);*/

#ifdef SOCKET_DEBUG
    printf("getaddrinfo node=%s service=%s\n", node ? node : "null", service ? service : "null");
#endif

    struct linux_addrinfo *res_temp = NULL;

    int ret = getaddrinfo(node, service, NULL /*(struct addrinfo*)hints*/, (struct addrinfo **)&res_temp);
    if (ret < 0)
    {
        printf("getaddrinfo error node=%s service=%s errno=%d\n", node ? node : "null", service ? service : "null", errno);
        return -1;
    }

    if (res_temp) {
        translate_addrinfo_to_msys((struct addrinfo *)res_temp);

        if (res)
            *res = res_temp;
    } else { // sometimes cygwin seems to return null addrinfo when theres no host?, hacky
        printf("getaddrinfo has no value?\n");
        errno = EAI_NONAME;
        return -1;
    }

    return ret;
}

struct linux_hostent *jmp_gethostbyname(const char *name)
{
#ifdef SOCKET_DEBUG
    printf("gethostbyname(%s)\n", name);
#endif
    struct hostent *temp = gethostbyname(name);
    if (!temp)
    {
        printf("gethostbyname error name=%s errno=%d\n", name, errno);
        return NULL;
    }

    // WARNING: MEM LEAK
    struct linux_hostent *host = (struct linux_hostent *)malloc(sizeof(linux_hostent));
    host->h_name = (char *)temp->h_name;
    host->h_aliases = temp->h_aliases;
    host->h_addrtype = temp->h_addrtype;
    host->h_length = temp->h_length;
    host->h_addr_list = temp->h_addr_list;
    return host;
}

struct linux_servent *jmp_getservbyname(const char *name, const char *proto)
{
#ifdef SOCKET_DEBUG
    printf("getservbyname(%s)\n", name);
#endif
    struct servent *temp = getservbyname(name, proto);
    if (!temp)
    {
        printf("getservbyname error name=%s errno=%d\n", name, errno);
        return NULL;
    }

    // WARNING: MEM LEAK
    struct linux_servent *serv = (struct linux_servent *)malloc(sizeof(linux_servent));
    serv->s_name = temp->s_name;
    serv->s_aliases = temp->s_aliases;
    serv->s_port = temp->s_port;
    serv->s_proto = temp->s_proto;
    return serv;
}

Stubs GetNetdbStubs()
{
    return {
        DEF_STUB(getaddrinfo),
        
        DEF_STUB(gethostbyname),
        DEF_STUB(getservbyname),};
}