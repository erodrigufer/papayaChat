#include "inet_sockets.h"
#include "basics.h"

#define SERVICE "51000" // port number
#define HOST "payaserver" // server IP
#define BUF_SIZE 4096

static int
getGreetingsMessage(int server_fd)
{

	ssize_t bytesRead;
	char string_buf[BUF_SIZE];

	while((bytesRead = read(server_fd, string_buf, BUF_SIZE)) > 0){
		printf("%s",string_buf); /* later implement this with write
								 directly to stdout */
	}
	if(bytesRead == -1)
		errExit("read greetings message");

	return 0;
}

static void 
handleWriteSocket(int server_fd){

	ssize_t bytesStdin;
	char userInput[BUF_SIZE];

	for(;;){
		while((bytesStdin = read(STDIN_FILENO, userInput,BUF_SIZE)) > 0){
			if (write(server_fd, userInput, bytesStdin) != bytesStdin)
				errExit("write to server");
		}
		if(bytesStdin == -1)
			errExit("read from server");
	}

}

static void
handleReadSocket(int server_fd){

	ssize_t bytesRead;
	char string_buf[BUF_SIZE];

	/* infinte for-loop reading from the TCP-socket
	every time a whole message is read, then read() returns 0
	and the while-loop will then re-start with a blocking read()
	until some bytes can be read from the socket */
	for(;;){
		while((bytesRead = read(server_fd, string_buf, BUF_SIZE)) > 0){
			printf("%s",string_buf); /* later implement this with write
									 directly to stdout */
		}
		if(bytesRead == -1)
			errExit("read from server");
	}
}

/* the parameter of the function is a function pointer to
the function that should be run by the child process:
either handleReadSocket or handleWriteSocket */
static void
createChildProcess(void functionChild(int), int server_fd){
	pid_t pidChild;
	pidChild = fork();
	switch(pidChild) {
	
	/* error on fork() call */
	case -1:
		/* exit with an error message, if fork fails then terminate 
		all child's processes, since all processes are vital */
		errExit("fork(), child could not be created.");
		break; /* break out of switch-statement*/

	/* Child process (returns 0) */
	case 0:
		printf("Intializing child process\n");
		/* function pointer to function of child process */
		functionChild(server_fd);
		break; /* break out of switch-statement*/
			
	/* Parent: fork() returns PID of the newly created child */
	default:
		printf("PID Child %ld\n",(long)pidChild);
		break; /* break out of switch-statement*/
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

	
	createChildProcess(handleReadSocket,server_fd);
	//getGreetingsMessage(server_fd);
	
	exit(EXIT_SUCCESS);

}
