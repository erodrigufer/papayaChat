/* inet_sockets.h

*/
#ifndef INET_SOCKETS_H
#define INET_SOCKETS_H          /* Header guard */

#include <sys/socket.h>
#include <netdb.h>


/* Create a socket with the given socket 'type', and connect it to the
address specified by 'host' and 'service'. It can handle both TCP and 
UDP clients, that connect their sockets to a server */
int inetConnect(const char *host, const char *service, int type);

int inetListen(const char *service, int backlog, socklen_t *addrlen);

int inetBind(const char *service, int type, socklen_t *addrlen);

char *inetAddressStr(const struct sockaddr *addr, socklen_t addrlen,
                char *addrStr, int addrStrLen);

#define IS_ADDR_STR_LEN 4096
                        /* Suggested length for string buffer that caller
                           should pass to inetAddressStr(). Must be greater
                           than (NI_MAXHOST + NI_MAXSERV + 4) */
#endif

/* Eduardo Rodriguez 2021 (c) (@erodrigufer) with some code taken and modified from Michael Kerrisk. Licensed under GNU GPLv3 */
