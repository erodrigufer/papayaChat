#include <syslog.h>				/* server runs as a daemon, logging handled
								through syslog API */
#include "configure_syslog.h" /* configure syslog to be able to add log at termination */
#include "basics.h"

int main(int argc, char *argv[]){

	configure_syslog();
	
	/* SIGTERM is the default signal sent to a process when the 'kill' command is used 
	in the terminal (when no other signal is specified). Use this signal to kill the 
	parent/listening server! */
	syslog(LOG_DEBUG, "SIGTERM signal received. Killing server backend [parent process]!");
	exit(EXIT_SUCCESS);

}
