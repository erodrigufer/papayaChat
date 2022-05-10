/* error_handling.h

TODO: Only the functions with a description here have being commented
in the .c file

*/
#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H		/* Header guard */

/* Display error message including 'errno' diagnostic,
return to caller without terminating process */
void errMsg(const char *format, ...);

/* only if the compiler is gcc, the following macros will be defined
In other words, one can use GNU extensions/attributes */
#ifdef __GNUC__

    /* This macro stops 'gcc -Wall' complaining that "control reaches
       end of non-void function" if we use the following functions to
       terminate main() or some other non-void function. */

	/* compiler optimization attribute
	   check for more info: 
	   https://gcc.gnu.org/onlinedocs/gcc-3.2/gcc/Function-Attributes.html */
	#define NORETURN __attribute__ ((__noreturn__))
	#else
	
	#define NORETURN /* if we are not using GCC then NORETURN will just be
						a blank macro definition */
#endif

/* Display error message including 'errno' diagnostic, and
   terminate the process, core dump will take place if env 
   variable EF_COREDUMP is set beforehand */
void errExit(const char *format, ...) NORETURN ;

void err_exit(const char *format, ...) NORETURN ;

void errExitEN(int errnum, const char *format, ...) NORETURN ;

/* Prints an error message without 'errno' diagnostic,
and termiantes process (core dump) */
void fatal(const char *format, ...) NORETURN ;

void usageErr(const char *format, ...) NORETURN ;

void cmdLineErr(const char *format, ...) NORETURN ;

#endif

/* Eduardo Rodriguez 2021 (c) (@erodrigufer) with some code taken and modified from Michael Kerrisk. Licensed under GNU AGPLv3 */
