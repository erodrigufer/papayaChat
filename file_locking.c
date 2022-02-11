/* file_locking.c

Use these functions to avoid race conditions, when writing or reading with multiple
processes from a single file.

The server back-end creates a child process for each client being served. These child processes
can asynchronously read and write to a central unique chat log file. Therefore, it is 
imperative to avoid race conditions when handling that chat log file. 

*/

/* Required to send SIGUSR1 signal, after finishing exclusive write() */
#include <signal.h>

/* Required for open(2) 
(next three headers) */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
/* In some UNIX implementations (those based on BSD) flock is done through
POSIX/fcntl file locking. */

/* Required for flock(2) (not BSD Linux distros) */
#include <sys/file.h>

#include "basics.h"

/* CONFIG.h header file includes the path where the central chat log file
will be stored; defined under CHAT_LOG_PATH as a string */
#include "CONFIG.h"

/* this is just an estimate of what the maximum number of characters per line
in the chatlog file could be */
#define MAX_CHARACTERS_PER_LINE 150

/* this is the maximum number of lines that we want to send back to the client
after the connection is established with the server for the first time, in other
words the last X lines of the file to be sent to the client */
#define LINES_SEND_BACK_TO_CLIENT 12

/* max amount of characters that can be sent back to the client */
#define MAX_CHARACTERS_BACK_CLIENT (MAX_CHARACTERS_PER_LINE * LINES_SEND_BACK_TO_CLIENT)

/* open the central chat log file
If file does not exist, it creates the file.
It returns fd of file if file is created or opened correctly.
If not, it returns -1 */
int
openChatLogFile(void)
{

	/* Define flags for open(2)
	-open for READ/WRITE
	-if file does not exist, CREATE file
	-when writting, always APPEND to file (atomic call)
	-CLOSE file descriptor on EXEC 
	*/
	int flags = O_RDWR | O_CREAT | O_APPEND | O_CLOEXEC ;

	/* File permissions (when file is created) */
	mode_t createPermissions = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;

	/* return fd of openned file, or -1 if error */
	return open(CHAT_LOG_PATH,flags,createPermissions);

}

/* place an exclusive lock and write to the file 
size_t sizeString is the size of the string to write to the file 
TODAY I LEARNED: size_t is for non-negative numbers, so the actual size
of a file; ssize_t also supports negative numbers, for example in a call
to read(2), since if the call fails it will return -1 */
int
exclusiveWrite(int file_fd, char* string, size_t sizeString)
{

	/* place an exclusive lock, if the file has any other lock
	(either exclusive or shared lock) then this call is going to block
	only when all other locks are liberated, this call will go through
	and we can be sure that this process is the only one writing to the file
	and no other file is reading at the same time (shared lock) */	
	if(flock(file_fd,LOCK_EX)==-1)
		return -1;

	if (write(file_fd, string, sizeString) != sizeString)
		return -1;

	/* send SIGUSR1 signal to process group, to signal in a MULTICAST way that 
	there are new messages in the chat log file
	the first argument is 0, so that the signal is sent to all members of the 
	process group */
    if(kill(0,SIGUSR1)==-1)
		return -1;

	/* unlock file */
	if(flock(file_fd,LOCK_UN)==-1)
		return -1;

	return 0;

}

/* place a shared lock, read from chat log file, store messages in string, which
will be sent to client */
int 
sharedRead(int file_fd, char* string, size_t sizeString, off_t offset)
{

	/* place a shared lock on chat log file, multiple process will be able to read
	concurrently from the file, but no writes are permitted (LOCK_EX) exclusive locks */
	if(flock(file_fd,LOCK_SH)==-1)
		return -1;

	ssize_t bytesRead;	/* bytes read from chat log file */

	int whence = SEEK_SET;	/* SEEK_SET= set the offset in relationship to the beginning of the file */ 

	/* set file offset in relationship to beginning of file */
	if(lseek(file_fd,offset,whence)==-1)
		return -1;

	bytesRead = read(file_fd,string,sizeString);
	if(bytesRead < 0)
		return -1;

	/* unlock file */	
	if(flock(file_fd,LOCK_UN)==-1)
		return -1;

	/* return the amount of bytesRead to move the offset further */
	return bytesRead;

}

/* lock the chatlog file, find out which bytes to send to the client to 
send a maximum amount of lines, send the lines to the client and unlock chatlog file 
returns -1 if there was an error, or the offset of the last byte of the file
if it was successful */
off_t
messagesFromFirstClientConnection(int file_fd, char* messages, size_t sizeMessages, int client_fd)
{
		/* place a shared lock on chat log file, multiple process will be able to read
	concurrently from the file, but no writes are permitted (LOCK_EX) exclusive locks */
	if(flock(file_fd,LOCK_SH)==-1)
		return -1;

	int whence = SEEK_END;	/* SEEK_END= set the offset at one byte after the last byte of the file */

	/* find the offset for the last byte of the file */
	off_t lastByteOfFile = lseek(file_fd,-1,whence);
	if(lastByteOfFile==-1){
		/* unlock file */	
		if(flock(file_fd,LOCK_UN)==-1)
			return -1;
		return -1;	/* there was an error while finding the last byte */
	}

	off_t workingOffset;
/* this checks if the size of the file is bigger than the possible biggest 
message back which one would be able to send, if not, check for newlines inside
the whole file, otherwise check only in a reduced area at the end of the file */
	if(lastByteOfFile < (MAX_CHARACTERS_BACK_CLIENT)){
		workingOffset = 0; /* beginning of file */
	}
	else{
		workingOffset = lastByteOfFile - (MAX_CHARACTERS_BACK_CLIENT) + 1 ;	/* add one, otherwise we read one extra character */
	}

	/* set file's offset at workingOffset in relationship to file's beginning */
	if(lseek(file_fd,workingOffset,SEEK_SET)==-1){
		/* unlock file */	
		if(flock(file_fd,LOCK_UN)==-1)
			return -1;
		return -1; /* error with lseek */
	}

	/* allocate memory to store text from chatlog file */
	char * chat_text = (char *) malloc(MAX_CHARACTERS_BACK_CLIENT);
	if(chat_text==NULL){
		/* unlock file */	
		if(flock(file_fd,LOCK_UN)==-1)
			return -1;
		return -1;	/* malloc failed */
	}
	ssize_t bytesRead;	/* bytes read from chat log file */
	bytesRead = read(file_fd,chat_text,MAX_CHARACTERS_BACK_CLIENT);
	if(bytesRead < 0){
		free(chat_text);
		/* unlock file */	
		if(flock(file_fd,LOCK_UN)==-1)
			return -1;
		return -1; /* read failed */
	}

	int newline_index=0;
	/* find first newline character, everything before the first newline, will not be sent back to the client */
	for(int i=0;i<bytesRead;i++){
		/* if true we found index of first newline */
		if(chat_text[i]=='\n'){
			/* start at one after newline */	
			newline_index=i+1;
			break;
		}		
	}// end for-loop
	
	/* count total amount of newlines in text, after first newline */
	int count = 0;
	for(int i=newline_index;i<bytesRead;i++){
		if(chat_text[i]=='\n')
			count++;
	}// end for-loop

	/* send the whole text back */
	if(count <= LINES_SEND_BACK_TO_CLIENT){
		/* send message to client socket */
		/* TODO: check if it is correct exactly what I am sending back to client, amount of bytes and index in array */
		if(write(client_fd,(char*)chat_text[newline_index],bytesRead-newline_index)!=bytesRead-newline_index){
			free(chat_text);
			/* unlock file */	
			if(flock(file_fd,LOCK_UN)==-1)
				return -1;
			return -1; /* write to socket failed */
		}
	}// end if count less or equal to LINES_SEND_BACK_TO_CLIENT

	/* send just the last 10 lines */
	if(count > LINES_SEND_BACK_TO_CLIENT){
		/* find the newline at total_new_lines - LINES_SEND_BACK_TO_CLIENT,
		and send everything from there until last byte of file which would be
		exactly the max amount of lines of text allowed (LINES_SEND_BACK_TO_CLIENT) */		
		int startTextIndex=newline_index;
		int count_newlines = 0;
		for(int i=newline_index;i<bytesRead;i++){
			if(chat_text[i]=='\n'){
				count_newlines++;
				if(count_newlines==(count-LINES_SEND_BACK_TO_CLIENT)){
					/* set index at 1 after the newline */
					startTextIndex=i+1;
					break;
				}
			}
		}//end for-loop

		/* send text to client */
		/* TODO: check if the index control is right */
		if(write(client_fd,(char*)chat_text[startTextIndex],bytesRead-startTextIndex)!=bytesRead-startTextIndex){
			free(chat_text);
			/* unlock file */	
			if(flock(file_fd,LOCK_UN)==-1)
				return -1;
			return -1; /* write to socket failed */
		}
	}// end if count bigger than LINES_SEND_BACK_TO_CLIENT

	/* unlock file */	
	if(flock(file_fd,LOCK_UN)==-1)
		return -1;

	free(chat_text);
	return lastByteOfFile;
}

/* Eduardo Rodriguez 2021 (c) (@erodrigufer). Licensed under GNU AGPLv3 */
