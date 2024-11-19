#include "whitelist.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/cygwin.h>

char WHITELIST_DIRECTORY[PATH_MAX];

bool isPathWhitelisted(const char *path)
{
    if (!path)
        return false;

    if (strstr(path, "/dev/") == path)
        return true;
    if (strstr(path, "/proc/") == path)
        return true;

    char *win = (char *)cygwin_create_path(CCP_POSIX_TO_WIN_A, path);
    if (!win)
        return false;

    //printf("[WHITELIST] Checking %s\n", win);

    if (strstr(win, WHITELIST_DIRECTORY) == win)
    {
        free(win);
        return true;
    }

    printf("[Whitelist] Path is not whitelisted %s\n", win);

    free(win);
    return false;
}

void fixPathIfNeeded(char* out, const char* path) {
    // if it starts with /tmp/ then replace it to ./tmp/
    if (strstr(path, "/tmp/") == path) {
        snprintf(out, PATH_MAX, ".%s", path);
        return;
    }
    snprintf(out, PATH_MAX, "%s", path);
}

void InitPathWhitelist()
{
    char cwd[PATH_MAX];
    if (!getcwd(cwd, sizeof(cwd)))
    {
        printf("[Whitelist] Error getting current directory\n");
        throw 1;
    }

    char *win = (char *)cygwin_create_path(CCP_POSIX_TO_WIN_A, cwd);
    if (!win || strlen(win) < 1)
    {
        printf("[Whitelist] Failed resolving current directory\n");
        throw 1;
    }

    printf("[Whitelist] Directory = %s\n", win);

    memcpy(WHITELIST_DIRECTORY, win, strlen(win) + 1);

    free(win);
}