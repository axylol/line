#pragma once

// whether to base address later
//#define ALLOC_LATER

#define MEMORY_ALIGN 0x1000

#define CUSTOM_STACK
// 16 MB
#define CUSTOM_STACK_SIZE 16777216

// Shit slow epoll implementation
#define SHIT_EPOLL

// mt3dx+ needs these
#define MUTEX_CHECK

// enable dlopen implemntation
//#define DLOPEN

// File whitelisting/sandboxing
#define AGGRESIVE_FILE_CHECK

// Debug
#define SOCKET_DEBUG

// BETA: Redirects /tmp to a folder in local directory, useful for 3dx+
#define ENV_REDIRECTION