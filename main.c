#include "basics.h"
#include "daemonCreation.h"

/* add sys logging capabilities to daemon */
#include <syslog.h>

/* libraries needed to prnt the pid of a process,
both libraries are already included in "basics.h" */
//#include <sys/types.h>
//#include <unistd.h>

/*
This is how the getpid() function is declared,
this functions always work (see documentation man 2 getpid [getpid is a kernel system call
so it is found on (2)])
       pid_t getpid(void);
       pid_t getppid(void);
*/
int configureSysLog(void){

	/* identityString: this const char* will be appended to all log messages */
	const char *identityString = "testDaemon";

	/* specify options for syslog 
	LOG_CONS: if there is an error sending to the system logger, then write the log
	message to the system console at /dev/console 
	LOG_PID: log the caller's PID to the log message as well */
	int logOptions = LOG_CONS | LOG_PID;

	int logFacility = LOG_USER; /* messages generated by random user processes */

	/* open a syslog with specified configuration */
	openlog(identityString, logOptions, logFacility);

	return 0;
}
int main(int argc, char *argv[]){

	daemonCreation(0);

/* configure the syslog API */
	if(configureSysLog() == -1)
		exit(EXIT_FAILURE); /* at this point stderr was probably redirected to 
		/dev/null so there is no point on writing an error message */

/*
int snprintf(char *str, size_t size, const char *format, ...);
Function definition to 
*/

	pid_t daemonpid = getpid(); /* declare daemonpid to print on syslog */
	pid_t daemonppid = getppid(); /* get daemonppid */
	/* cast pid_t to a long inside the snprintf call, since there is no specifier for pid_t */ 
	
	/* Print the Daemon's PID and PPID into syslog. The PPID should be 1, since the parent of an 
	orphaned process gets to be PID 1 (systemd or init) */
	syslog(LOG_INFO, "Daemon's PID: %ld. Daemon's PPID: %ld.", (long) daemonpid, (long) daemonppid);

	closelog();

	exit(EXIT_SUCCESS);
}



/* Eduardo Rodriguez 2021 (c) (@erodrigufer) with some code taken and modified from Michael Kerrisk. Licensed under GNU GPLv3 */
