/* daemonCreation.c

	Make a process a daemon with this function

	Licensing and copyright at the end of file.

*/


#include <sys/stat.h>
#include <fcntl.h>
#include "daemonCreation.h" /* local header for this file */
#include "mh.h"
// comment out the library from the book (although it might be necessary for some functions)
// #include "tlpi_hdr.h"

int                                     /* Returns 0 on success, -1 on error */
daemonCreation(int flags)
{
    int maxfd, fd;

    switch (fork()) {                   /* Become background process */
    	case -1: return -1;								/* Problem with fork() syscall */
    	
			case 0:  break;                     /* Child falls through... */
			/* break call exits from switch cases, so that no other switch case 
			could theoretically be executed */

    	default: _exit(EXIT_SUCCESS);       /* parent process terminates */
			/* Parent process has to terminate so that the process that will become the daemon,
			will never get to be the Process Group Leader. If the process is not the PGL, then it
			is not possible for it to have a controlling terminal (exactly this behaviour is 
			desirable for a daemon)*/
    } // end switch case fork

    if (setsid() == -1)                 /* Become leader of new session */
        return -1;

    switch (fork()) {                   /* Ensure we are not session leader */
    	case -1: return -1; /* There was a problem with the syscall */
    	
			case 0:  break; /* This child process will be our daemon, which cannot be the session leader */
    	
			default: _exit(EXIT_SUCCESS); /* session leader exits */
    }

 /* Only the child of the leader of the new session has come this far.
 Compare the flags passed to the function call of daemonCreation() with the flags
 stored in the header daemonCreation.h. If the flags are equal then the commands in the if-statement
 are not executed, since the comparison is being negated. */
    if (!(flags & DAEMON_FLAG_NO_UMASK0))
        umask(0);                       /* Clear file mode creation mask */

    if (!(flags & DAEMON_FLAG_NO_CHDIR))
        chdir("/");                     /* Change to root directory */

    if (!(flags & DAEMON_FLAG_NO_CLOSE_FILES)) { /* Close all open files */
        maxfd = sysconf(_SC_OPEN_MAX); /* store in maxfd, the max number of open file descriptors
				that can be in the system */
        if (maxfd == -1)                /* Limit is indeterminate... */
            maxfd = DAEMON_FLAG_MAX_CLOSE;       /* so take a guess */

				/* close all file descriptors up to the max file descriptor numer */
        for (fd = 0; fd < maxfd; fd++)
            close(fd);
    } // end if-DAEMON_FLAG_NO_CLOSE_FILES

    if (!(flags & DAEMON_FLAG_NO_REOPEN_STD_FDS)) {
        close(STDIN_FILENO);            /* Reopen standard fd's to /dev/null */

        fd = open("/dev/null", O_RDWR);

        if (fd != STDIN_FILENO)         /* 'fd' should be 0, STDIN is fd 0 normally */
            return -1;
        if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO)
            return -1;
        if (dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO)
            return -1;
    }

    return 0;
}

/*
License and copyright notice
Originally taken from Michael Kerrisk, extensive modifications and comments from Eduardo Rodriguez (@erodrigufer)
Licensed under GNU GPLv3
*/
