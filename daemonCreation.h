/* daemonCreation.h

*/
#ifndef DAEMONCREATION_H             /* Header guard */
#define DAEMONCREATION_H             

/* Bit-mask values for 'flags' argument of daemonCreation() 
Description: 
This are self-defined macros (or flag values) to be used as inputs with the 'flags' argument 
for the function daemonCreation()
*/

#define DAEMON_FLAG_NO_CHDIR           01    /* Don't chdir("/"), do not change the working directory to '/' */
#define DAEMON_FLAG_NO_CLOSE_FILES     02    /* Don't close all open files */
#define DAEMON_FLAG_NO_REOPEN_STD_FDS  04    /* Don't reopen stdin, stdout, and
                                       		 stderr to /dev/null */
#define DAEMON_FLAG_NO_UMASK0         010    /* Don't do a umask(0) */

#define DAEMON_FLAG_MAX_CLOSE  8192          /* Maximum file descriptors to close if
                                       sysconf(_SC_OPEN_MAX) is indeterminate */

int daemonCreation(int flags);

#endif


/*
License and copyright notice
Originally taken from Michael Kerrisk, extensive modifications and comments by Eduardo Rodriguez (@erodrigufer)
Licensed under GNU GPLv3
*/
