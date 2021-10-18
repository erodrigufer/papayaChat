#include "inet_sockets.h"
#include "basics.h"

#define SERVICE "51000" // port number
#define HOST "payaserver" // server IP

int
main(int argc, char *argv[])
{

	int server_fd; /* fd for server connection */

	/* SOCK_STREAM for TCP connection */
	server_fd = clientConnect(HOST,SERVICE,SOCK_STREAM);
	if(server_fd == -1)
		errExit("clientConnect"); /* connection failed, exit */

	exit(EXIT_SUCCESS);

}
