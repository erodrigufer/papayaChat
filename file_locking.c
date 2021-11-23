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

	return open(CHAT_LOG_PATH,flags,createPermissions);

}


/* Eduardo Rodriguez 2021 (c) (@erodrigufer). Licensed under GNU AGPLv3 */
