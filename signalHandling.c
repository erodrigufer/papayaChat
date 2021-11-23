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

}


void
configureSignalDisposition(void)
{

    struct sigaction sa_sigchild;			/* struc is necessary to define signals mask
											to be blocked during signal handler, needed 
											for syscall sigaction*/

    /* Establish SIGCHLD handler to reap terminated child processes,
	if SIGCHLD is gathered with waitpid() or wait() by parent, then child
	process becomes a zombie and resources (PIDs) are not used efficiently.
	SIGCHLD is sent by the kernel to a parent process when one of its childern terminates
	(either by calling exit() or as a result of being killed by a signal).

	sa_mask is the signal set of signals that would be blocked during the
	invocation of the handler
	-> create an empyte signal set, no signal blocked during invocation of handler */
    sigemptyset(&sa_sigchild.sa_mask);			

	/* if a syscall is interrupted by the SIGCHLD, the kernel should
	restart the syscall after handling the signal,
	for that the SA_RESTART flag is used. Not all syscalls can be
	properly restarted by the kernel, check 21.5 of 'The Linux 
	Programming Interface' 
	for the SIGTERM handler this is not required, since all kernel syscalls
	interrupted will not matter since the process should exit ASAP */
    sa_sigchild.sa_flags = SA_RESTART;
	/* catchSIGCHLD is the function handler for a SIGCHLD signal */
    sa_sigchild.sa_handler = catchSIGCHLD;	

	/* the new disposition for SIGCHLD signal is the grimReaper function, the old
	signal disposition is not stored anywhere (NULL) */
    if (sigaction(SIGCHLD, &sa_sigchild, NULL) == -1)
		errExit("sigaction for SIGCHLD");

}

/* Eduardo Rodriguez 2021 (c) (@erodrigufer). Licensed under GNU AGPLv3 */
