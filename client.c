#include "inet_sockets.h"
#include "basics.h"

#define SERVICE "51000" // port number
#define HOST "payaserver" // server IP


static int
getGreetingsMessage(int server_fd)
{

	#define BUF_SIZE 4096
	ssize_t bytesRead;
	char string_buf[BUF_SIZE];

	while((bytesRead = read(server_fd, string_buf, BUF_SIZE)) > 0){
		printf("%s",string_buf); /* later implement this with write
								 directly to stdout */
	}

	return 0;
}

static void 
handleWriteSocket(void){

}

static void
handleReadSocket(void){

}

static void
createChildProcess(void){
	switch(fork()) {
	
	/* error on fork() call */
	case -1:
		/* exit with an error message, if fork fails then terminate 
		all child's processes, since all processes are vital */
		errExit("fork(), child could not be created.");

	/* Child process (returns 0) */
	case 0:
		/* add function pointer to function of child process */
	
	/* Parent: fork() returns PID of the newly created child */
	default:
		/* add parent's functions */
	} // end switch-statement
}

int
main(int argc, char *argv[])
{

	int server_fd; /* fd for server connection */

	/* SOCK_STREAM for TCP connection */
	server_fd = clientConnect(HOST,SERVICE,SOCK_STREAM);
	if(server_fd == -1)
		errExit("clientConnect"); /* connection failed, exit */


	getGreetingsMessage(server_fd);
	
	exit(EXIT_SUCCESS);

}
