/* inet_sockets.c

   Package with configuration functions for TCP/IP sockets in C

*/
#define _BSD_SOURCE             /* To get NI_MAXHOST and NI_MAXSERV
                                   definitions from <netdb.h> */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "inet_sockets.h"       /* Declares functions defined here */
#include "basics.h"

/* Common arguments to the functions of this library:

        (const char *)'host':         NULL for loopback IP address, or
                        			 a host name or numeric IP address
									 (both IPv4 and IPv6)

        (const char*) 'service':      either a name or a port number
        (int) 'type':         		either SOCK_STREAM or SOCK_DGRAM
*/

/* Create a socket with the given socket 'type', and connect it to the
address specified by 'host' and 'service'. It can handle both TCP and 
UDP clients, that connect their sockets to a server 
Returns a file descriptor on success, or a -1 on error */
int
inetConnect(const char *host, const char *service, int type)
{
    struct addrinfo hints;
	/* store getaddrinfo() results*/
    struct addrinfo *addr_results;
	/* iterate through the linked list of results
	found with getaddrinfo() */
	struct addrinfo *possible_addr;
    int sfd, s;

	/* use memset to guarantee that all values inside the addrinfo struct
	are initialized with a 0 */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    hints.ai_family = AF_UNSPEC;        /* Allows IPv4 or IPv6 */
    hints.ai_socktype = type;			/* can be either SOCK_STREAM or SOCK_DGRAM
										to handle TCP and UDP */

	/* man 3 getaddrinfo
	(From the man page) Given  host  and  service,  which  identify an Internet host and a service, getaddrinfo() returns one or more addrinfo structures, each of which contains an Internet  address that can be specified in a call to bind(2) or connect(2).

	Store the possible results in the addrinfo struct pointer at addr_results
	*/
    s = getaddrinfo(host, service, &hints, &addr_results);
    if (s != 0) { /* getaddrinfo() returns 0 on success */
        errno = ENOSYS; /* still not sure
		why pick ENOSYS?? eliminate this errno change after testing!! */
        return -1; /* getaddrinfo() failed so end inetConnect with -1 */
    }

    /* Walk through returned list until we find an address structure
       that can be used to successfully connect a socket 
	   Stop when possible_addr points to NULL (no more addresses in the structure [linked 
	   list] */
    for (possible_addr = addr_results; possible_addr != NULL; possible_addr = possible_addr->ai_next) {
        sfd = socket(possible_addr->ai_family, possible_addr->ai_socktype, possible_addr->ai_protocol);
        if (sfd == -1)
            continue;                   /* On error, try next address */

        if (connect(sfd, possible_addr->ai_addr, possible_addr->ai_addrlen) != -1)
            break;                      /* Success */

        /* Connect failed: close this socket and try next address */
        close(sfd);
    }

    freeaddrinfo(addr_results);

    return (possible_addr == NULL) ? -1 : sfd;
}

/* Create an Internet domain socket and bind it to the address
   { wildcard-IP-address + 'service'/'type' }.
   If 'doListen' is TRUE, then make this a listening socket (by
   calling listen() with 'backlog'), with the SO_REUSEADDR option set.
   If 'addrLen' is not NULL, then use it to return the size of the
   address structure for the address family for this socket.
   Return the socket descriptor on success, or -1 on error. */

static int              /* Public interfaces: inetBind() and inetListen() */
inetPassiveSocket(const char *service, int type, socklen_t *addrlen,
                  Boolean doListen, int backlog)
{
    struct addrinfo hints;
    struct addrinfo *addr_results, *rp;
    int sfd, optval, s;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    hints.ai_socktype = type;
    hints.ai_family = AF_UNSPEC;        /* Allows IPv4 or IPv6 */
    hints.ai_flags = AI_PASSIVE;        /* Use wildcard IP address */

    s = getaddrinfo(NULL, service, &hints, &addr_results);
    if (s != 0)
        return -1;

    /* Walk through returned list until we find an address structure
       that can be used to successfully create and bind a socket */

    optval = 1;
    for (rp = addr_results; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue;                   /* On error, try next address */

        if (doListen) {
            if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval,
                    sizeof(optval)) == -1) {
                close(sfd);
                freeaddrinfo(addr_results);
                return -1;
            }
        }

        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;                      /* Success */

        /* bind() failed: close this socket and try next address */

        close(sfd);
    }

    if (rp != NULL && doListen) {
        if (listen(sfd, backlog) == -1) {
            freeaddrinfo(addr_results);
            return -1;
        }
    }

    if (rp != NULL && addrlen != NULL)
        *addrlen = rp->ai_addrlen;      /* Return address structure size */

    freeaddrinfo(addr_results);

    return (rp == NULL) ? -1 : sfd;
}

/* Create stream socket, bound to wildcard IP address + port given in
  'service'. Make the socket a listening socket, with the specified
  'backlog'. Return socket descriptor on success, or -1 on error. */

int
inetListen(const char *service, int backlog, socklen_t *addrlen)
{
    return inetPassiveSocket(service, SOCK_STREAM, addrlen, TRUE, backlog);
}

/* Create socket bound to wildcard IP address + port given in
   'service'. Return socket descriptor on success, or -1 on error. */

int
inetBind(const char *service, int type, socklen_t *addrlen)
{
    return inetPassiveSocket(service, type, addrlen, FALSE, 0);
}

/* Given a socket address in 'addr', whose length is specified in
   'addrlen', return a null-terminated string containing the host and
   service names in the form "(hostname, port#)". The string is
   returned in the buffer pointed to by 'addrStr', and this value is
   also returned as the function result. The caller must specify the
   size of the 'addrStr' buffer in 'addrStrLen'. */

char *
inetAddressStr(const struct sockaddr *addr, socklen_t addrlen,
               char *addrStr, int addrStrLen)
{
    char host[NI_MAXHOST], service[NI_MAXSERV];

    if (getnameinfo(addr, addrlen, host, NI_MAXHOST,
                    service, NI_MAXSERV, NI_NUMERICSERV) == 0)
        snprintf(addrStr, addrStrLen, "(%s, %s)", host, service);
    else
        snprintf(addrStr, addrStrLen, "(?UNKNOWN?)");

    return addrStr;
}

/* Eduardo Rodriguez 2021 (c) (@erodrigufer) with some code taken and modified from Michael Kerrisk. Licensed under GNU AGPLv3 */
