/* CONFIG.h

Default config file

This file defines IP and port of chat service server 
- SERVICE is the PORT number
- HOST is the server's IP

It also defines the path to store the central chat log file
CHAT_LOG_PATH
*/

#ifndef CONFIG_H /* add header guard */
#define CONFIG_H

/* pathname of program to handle sigterm handler in async-safe way, 
execve to this program from inside signal handler 
[used in concurrent_server.c] */
/* the binaries to test network connectivity are compiled with different paths
than the binaries used in the installation of the actual daemon */
#ifndef TEST
#define PATHNAME_TERM_ASYNC_SAFE "/usr/local/bin/papayachat/termHandlerAsyncSafe.bin" 
#else
#define PATHNAME_TERM_ASYNC_SAFE "./termHandlerAsyncSafe.bin" 
#endif

/* bytes transmission size, defined in CONFIG.h
to share the value between multiple files */
#define BUF_SIZE 4096 

/* [back-end] max number of clients in listening backlog queue */
#define BACKLOG_QUEUE 10		

/* try to read at most MAX_LINE_LENGTH characters per line when parsing a config file */
#define MAX_LINE_LENGTH 512

/* path of config file for client */
#define CLIENT_CONFIG_PATH "/etc/papayachat/client.config"

/* if compiled with gcc -D NON_DEFAULT_CONFIG option, then only userConfig.h
will be used */
#ifdef NON_DEFAULT_CONFIG
#include "userConfig.h"
#endif

/* ------------------------------------------------------------------------ */
#ifndef NON_DEFAULT_CONFIG
/* IMPORTANT:
These are the values, that are also represented in userConfig.h if the 
file exists, append any other values only present in the general (default)
config file outside this #ifndef part 
*/

/* Define TCP/IP services */
#define SERVICE "51000" /* Port to connect to with server */
#define HOST "localhost" /* server address (found, e.g. on /etc/hosts file) */

/* the binaries to test network connectivity are compiled with different paths
than the binaries used in the installation of the actual daemon */
#ifndef TEST
/* Define path of chat log file */
#define CHAT_LOG_PATH "/var/lib/papayachat/papayachat.chat"
#else
#define CHAT_LOG_PATH "./chat_log.chat"
#endif

#endif /* endif for NON_DEFAULT_CONFIG */
/* ------------------------------------------------------------------------ */

#endif /* endif for header guard */
