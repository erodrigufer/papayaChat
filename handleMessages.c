/* handleMessages.c

*/

#include "CONFIG.h" /* BUF_SIZE is defined here */
#include "basics.h" /* to use read() write() */

/* send a message to a pipe */
void
sendMessageToPipe(int pipe_fd, char *message)
{
	/* Execute strlen outside of write(), since it demands run time resources
	and it would be run twice inside write() call*/
	size_t sizeOfMessage = strlen(message);
	/* it writes sizeOfMessage + 1, in order to also send a '0' after the message */
	if(write(pipe_fd,message,sizeOfMessage+1)!=sizeOfMessage+1)
		errExit("write sendMessageToPipe()");

}

/* send a message to the server_fd, the message is received through 
a pipe from the parent process */
void
handleSendSocket(int server_fd, int pipe_fd)
{
	/* size of info read from pipe */
	ssize_t bytesRead;

	for(;;){

		/* allocate memory on each for-loop to read message
		from pipe */
		char * string_buf = (char *) malloc(BUF_SIZE);
		/* if malloc fails, it returns a NULL pointer */
		if(string_buf == NULL)
			errExit("malloc failed. @handleSendSocket()");

		/* read from pipe info to send to server */
		bytesRead = read(pipe_fd, string_buf, BUF_SIZE);
/*----------- error handling for read()----------------------------------- */
		/* read() failed, exit programm with error, always free malloc resources
		before exit()*/
		if(bytesRead == -1){
			free(string_buf);
			errExit("read - handleSendSocket()");
		}
		/* connection to pipe closed */
		if(bytesRead == 0){
			free(string_buf);
			errExit("pipe closed - handleSendSocket()");
		}
/*----------- error handling for read()----------------------------------- */
		/* send data received from pipe to server socket */
		if(write(server_fd,string_buf,bytesRead)!=bytesRead)
			errExit("write handleSendSocket()");

		/* free resources, char * buffer to read message from pipe */
		free(string_buf);
	} // end for-loop

}

/* fetch messages from pipe (fetch, but do not print message yet)
the pipe should be configured as O_NONBLOCK
since if the read() on the pipe blocks, all the CLI stalls */
int
fetchMessage(int pipe_fd, char *string_buf)
{
	ssize_t bytesRead;

	bytesRead = read(pipe_fd, string_buf, BUF_SIZE);

/* TODO: handle errors from read() */

	/* reading from pipe failed */
//	if(bytesRead == -1)
		//errMsg("read greetings message");

	return bytesRead;

}

/* read from server socket and pass data through pipe to frontEnd parent process */
void
handleReadSocket(int server_fd, int pipe_fd)
{

	ssize_t bytesRead;

/* the right way to handle strings is by allocating new memory each time
a string is received through a read() call from the server, since 
char * should be immutable objects in C */
	for(;;){

		/* TODO: check if the size allocated is correct */
		/* allocate memory on each for-loop to read from server, into
		newly allocated char* buffer */
		char * string_buf = (char *) malloc(BUF_SIZE);
		/* if malloc fails, it returns a NULL pointer */
		if(string_buf == NULL)
			errExit("malloc() failed. @handleReadSocket()");
		
		bytesRead = read(server_fd, string_buf, BUF_SIZE);
/*----------- error handling for read()----------------------------------- */
		/* read() failed, exit programm with error, always free malloc resources
		before exit()*/
		if(bytesRead == -1){
			free(string_buf);
			errExit("read from server @handleReadSocket()");
		}
		/* connection to server down */
		if(bytesRead == 0){
			free(string_buf);
			errExit("connection to server lost! read() from socket return 0 == EOF :@handleReadSocket()");
		}
/*----------- error handling for read()----------------------------------- */
		/* send data received from server to parent process through pipe */
		if(write(pipe_fd, string_buf, bytesRead) != bytesRead){
			/* the amount of bytes written is not equal to the amount of bytes read */
			errExit("write to pipe @handleReadSocket()");
		}
		/* free resources, char * buffer to read message from server */
		free(string_buf);
	}
}

/* Eduardo Rodriguez 2021 (c) (@erodrigufer). Licensed under GNU AGPLv3 */

