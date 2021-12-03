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
/* Eduardo Rodriguez 2021 (c) (@erodrigufer). Licensed under GNU AGPLv3 */
