/* signalHandling.h

Function to handle signals
*/

#ifndef SIGNALHANDLING_H
#define SIGNALHANDLING_H

/* grimReaper/catch SIGCHLD in parent process and avoid
zombie processes */
void catchSIGCHLD(int sig);

/* kill all child processes from within parent process */
void killChildProcesses(void);

/* configure the signal disposition to catch SIGCHLD signals */
void configureSignalDisposition(void);

#endif

/* Eduardo Rodriguez 2021 (c) (@erodrigufer). Licensed under GNU AGPLv3 */
