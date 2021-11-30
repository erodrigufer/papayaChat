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
#define PATHNAME_TERM_ASYNC_SAFE "./termHandlerAsyncSafe.bin" 

/* bytes transmission size, defined in CONFIG.h
to share the value between multiple files */
#define BUF_SIZE 4096 

/* [back-end] max number of clients in listening backlog queue */
#define BACKLOG_QUEUE 10		

/* if compiled with gcc -D NON_DEFAULT_CONFIG option, then only userConfig.h
will be used */
#ifdef NON_DEFAULT_CONFIG
#include "userConfig.h"
#endif

/* if userConfig.h exists, then USERCONFIG_H will be
defined, and the user-defined macros will be used, not the default
ones from CONFIG.h */

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

/* Define path of chat log file */
//#define CHAT_LOG_PATH "/var/lib/papayachat/papayachat.chat"
#define CHAT_LOG_PATH "./chat_log.chat"
#endif /* endif for USERCONFIG_H */
/* ------------------------------------------------------------------------ */

#endif /* endif for header guard */
