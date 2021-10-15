#include <syslog.h>				/* server runs as a daemon, logging handled
								through syslog API */
#include "basics.h"

int main(int argc, char *argv[]){
	
	/* SIGTERM is the default signal sent to a process when the 'kill' command is used 
	in the terminal (when no other signal is specified). Use this signal to kill the 
	parent/listening server! */
	syslog(LOG_DEBUG, "SIGTERM signal received. Killing process!");
	exit(EXIT_SUCCESS);
	
	/* this point should never be achieved, the return is added to avoid compiler
	messages */
	return 0;
}
