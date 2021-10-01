/* error_handling.h

*/
#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H		/* Header guard */

/* Error diagnostic routines */

void errMsg(const char *format, ...);

#ifdef __GNUC__

    /* This macro stops 'gcc -Wall' complaining that "control reaches
       end of non-void function" if we use the following functions to
       terminate main() or some other non-void function. */

#define NORETURN __attribute__ ((__noreturn__))
#else
#define NORETURN
#endif

void errExit(const char *format, ...) NORETURN ;

void err_exit(const char *format, ...) NORETURN ;

void errExitEN(int errnum, const char *format, ...) NORETURN ;

void fatal(const char *format, ...) NORETURN ;

void usageErr(const char *format, ...) NORETURN ;

void cmdLineErr(const char *format, ...) NORETURN ;

#endif

/* Eduardo Rodriguez 2021 (c) (@erodrigufer) with some code taken and modified from Michael Kerrisk. Licensed under GNU AGPLv3 */
