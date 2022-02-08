/* signalHandling.c  */

#include <signal.h>				/* check 'man 2 sigaction' signal.h is needed
								to change the disposition of signals with
								the sigaction() syscall */	
#include <sys/wait.h>	/* wait on child processes */
#include "basics.h" /* include library to handle errors */
#include "signalHandling.h"


/* in order to test the functionality, we are just going to change the state of a global variable */
volatile sig_atomic_t flag_activated;

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
	
	/* outside of while-loop */
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
/* errMsg is not going to be displayed if this is running as a terminalless daemon*/
		errMsg("Failed to kill processes in Process Group!");

}

/* if configuration successful it returns 0, if any syscall fails
it returns -1, so that the caller-function can handle the error,
logging it and exiting 
configures the SIGCHLD and SIGUSR1 signals for the parent process */
int
configureSignalDisposition(void)
{


	struct sigaction sa_sigusr1;		/* configure the system to ignore the SIGUSR1 signal used
										by the child processes to communicate that a message was written
										in the chatlog and should now be sent to all clients */
	/* EXPLANATION: the parent process which is listening for new clients should completely ignore
	the SIGUSR1 signal used by some of its child processes, because it would otherwise terminate immediately if 
	it receives this signal without a user-defined signal disposition */

	/* use the SIG_IGN constant as signal handler to simply ignore this signal
	the process will not even get notified by the kernel, when the signal is sent to a process group
	it is then not necessary to define flags or a signals mask */
	sa_sigusr1.sa_handler = SIG_IGN;
	if (sigaction(SIGUSR1, &sa_sigusr1, NULL) == -1)
		return -1;	/* sigaction failed */ 



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
    if(sigemptyset(&sa_sigchild.sa_mask)==-1)
		return 1;

	/* block signal SIGUSR1, while inside the signal handler for SIGCHLD
	this call is maybe unnecessary, since SIGUSR1 is already being ignored 
	but the documentation is not clear if signals keep being ignored inside a signal handler */
	if(sigaddset(&sa_sigchild.sa_mask, SIGUSR1)==-1)
		return 1;

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

	/* the new disposition for SIGCHLD signal is the catchSIGCHLD() function, the old
	signal disposition is not stored anywhere (NULL) */
    if (sigaction(SIGCHLD, &sa_sigchild, NULL) == -1)
		return -1;	/* sigaction failed */
	
	return 0; /* exit successful */

}

void 
handlerSIGUSR1(int sig)
{
	flag_activated = 1;	

}

/* the processes that send messages from the chatlog to the clients must activate SIGUSR1 to be notified
by the processes that receive messages from the clients, when they have successfully performed an exlusive
write in the chatlog file 
return 0 when successful, and -1 on error */
int 
activateSIGUSR1(void)
{
	/* initial value */
	flag_activated = 0;
	struct sigaction sa_sigusr1;

	/* do not block any signals while inside signal handler for SIGUSR1, since e.g. a SIGTERM could take place
	in that moment */
	if(sigemptyset(&sa_sigusr1.sa_mask)==-1)
		return -1;

	/* the flags are explicitedly set to 0, because, otherwise SA_RESETHAND was being used in an Ubuntu cloud server
	which basically reset the handler for SIGUSR1, therefore killing the child process sending back the messages to the client
	after receiving the second message */		
	sa_sigusr1.sa_flags = 0;
	
	/* define the signal handler */
	sa_sigusr1.sa_handler = handlerSIGUSR1;

	if(sigaction(SIGUSR1, &sa_sigusr1, NULL) == -1)
		return -1;

	return 0; /* exit successful */

}

/* nothing happens inside SIGALRM handler */
void
timeoutHandler(int sig)
{
}

/* setup SIGALRM disposition for timeout during authentication,
if function fails it returns -1, otherwise 0 */
int 
configureTimeout(void)
{

	struct sigaction timeout;
	timeout.sa_flags = 0;	/* do not restart syscalls */
	/* do not block any signals inside handler for SIGALRM */
	sigemptyset(&timeout.sa_mask);
	timeout.sa_handler = timeoutHandler;
	if(sigaction(SIGALRM, &timeout, NULL)==-1)
		return -1;


}

/* Eduardo Rodriguez 2021 (c) (@erodrigufer). Licensed under GNU AGPLv3 */
