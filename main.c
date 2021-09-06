#include "basics.h"
#include "daemonCreation.h"

/* add sys logging capabilities to daemon */
#include <syslog.h>

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

	if(configureSysLog() == 1)
		exit(-1);

	sleep(40);	

	exit(EXIT_SUCCESS);
}



/* Eduardo Rodriguez 2021 (c) (@erodrigufer) with some code taken and modified from Michael Kerrisk GNU GPLv3 */
