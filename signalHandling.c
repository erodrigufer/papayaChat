#include <signal.h>				/* check 'man 2 sigaction' signal.h is needed
								to change the disposition of signals with
								the sigaction() syscall */	
#include <sys/wait.h>	/* wait on child processes */
#include "basics.h" /* include library to handle errors */

/* run this function to catch SIGCHLD of child processes exiting */
void
catchSIGCHLD(int sig)
{
	int savedErrno;             /* Save 'errno' in case changed here, errno
									can be changed by waitpid() if no more children exist
									then 'errno'= ECHILD */
		savedErrno = errno;
		while (waitpid(-1, NULL, WNOHANG) > 0)	/* 'man 2 waitpid' -1 means that it waits 
		for any of its child processes, the option WNOHANG makes the syscall waitpid()
		to return inmediately if no children has exited, if children exist but none has changed 
		status, then waitpid returns 0, so it exits the while-loop (on error, it returns -1 and
		also exits the while-loop). In any other case, multiple children have exited, so waitpid()
		will return their pids (which are larger than 0). After the last children which has changed
		status, waitpid will return either 0 or -1 and the while-loop will come to an end
		If there are no more children, then waitpid will return -1 and set errno to ECHILD */
			continue;
		errno = savedErrno;			/* restore errno to value before signal handler */

}

/* run this function to kill child processes */
void
killChildProcesses(void)
{
	/* IMPORTANT: I should not use errExit with these functions
	since they are already being run atexit(), so only display 
	diagnostic error messages with 'errMsg' 
	By using pid=0 I send the SIGTERM signal to every process in the
	Process Group (all children) */
	if(kill(0,SIGTERM)==-1)
		errMsg("Failed to kill processes in Process Group!");

/* TODO: debugging purpouses, remove fprintf later */
	fprintf(stderr,"Successfully killed child processes!\n");

}
