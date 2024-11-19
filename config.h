#pragma once

// whether to base address later
//#define ALLOC_LATER

#define MEMORY_ALIGN 0x1000

// FIXME: this is a hacky fix for mt4
#define CUSTOM_STACK
#define CUSTOM_STACK_SIZE 16777216

// Shit slow epoll implementation
#define SHIT_EPOLL

#define MUTEX_CHECK

// enable dlopen implemntation
//#define DLOPEN

// File whitelisting/sandboxing
#define AGGRESIVE_FILE_CHECK

// Debug
#define SOCKET_DEBUG

// Redirects /tmp to a folder in local directory, useful for 3dx+
#define ENV_REDIRECTION