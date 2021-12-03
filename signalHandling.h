/* signalHandling.h

Function to handle signals
*/

#ifndef SIGNALHANDLING_H	/* header guard */
#define SIGNALHANDLING_H


/* in order to test the functionality, we are just going to change the state of a global variable */
extern volatile sig_atomic_t flag_activated = 0;

/* grimReaper/catch SIGCHLD in parent process and avoid
zombie processes */
void catchSIGCHLD(int);

/* kill all child processes from within parent process */
void killChildProcesses(void);

/* configure the signal disposition to catch SIGCHLD signals */
int configureSignalDisposition(void);

/* configure the signal disposition to catch SIGUSR1 signals 
in the child processes that send messages back to the clients */
int activateSIGUSR1(void);

/* function handler for SIGUSR1 signal */
void handlerSIGUSR1(int);

#endif

/* Eduardo Rodriguez 2021 (c) (@erodrigufer). Licensed under GNU AGPLv3 */
