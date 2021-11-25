/* file_locking.c

Use these functions to avoid race conditions, when writing or reading with multiple
processes from a single file.

The server back-end creates a child process for each client being served. These child processes
can asynchronously read and write to a central unique chat log file. Therefore, it is 
imperative to avoid race conditions when handling that chat log file. 

*/

/* Required for open(2) 
(next three headers) */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
/* In some UNIX implementations (those based on BSD) flock is done through
POSIX/fcntl file locking. */
/* Required for flock(2) */
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
		//errExit("flock failed @ exclusiveWrite()");

	if (write(file_fd, string, sizeString) != sizeString) {
		//syslog(LOG_ERR, "write() failed: %s", strerror(errno));
		//_exit(EXIT_FAILURE);
		return -1;
	}// write()
	
	/* unlock file */
	if(flock(file_fd,LOCK_UN)==-1)
		return -1;

	return 0;

}

/* Eduardo Rodriguez 2021 (c) (@erodrigufer). Licensed under GNU AGPLv3 */
