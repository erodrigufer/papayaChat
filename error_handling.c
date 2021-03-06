/* error_handling.c

   Error handling routines 
*/
#include <stdarg.h>				/* A function may be called with a varying 
								number of arguments of varying types.  The 
								include file <stdarg.h> declares a type 
								(va_list) and defines three macros for 
								stepping through a list of arguments whose
								number and types are not known to the called function. */
#include "error_handling.h"		/* Custom-made error handling functions */
#include "basics.h"
#include "error_names.c.inc"    /* Defines error name codes and MAX_ENAME,
								MAX_ENAME define the max number of error codes */

/* it first checks if the compiler is GCC, GCC with -Wall flag will
complain that main function does not return if one of the error 
functions that does not return is called due to an error */
#ifdef __GNUC__                 /* Prevent 'gcc -Wall' complaining  */
__attribute__ ((__noreturn__))  /* if we call this function as last */
#endif                          /* statement in a non-void function */


/* defined as static, because only used inside this file! */
static void
terminate(Boolean useExit3)		/* useExit3 refers to using the glibc function
								exit() (see 'man 3 exit', instead of _exit() */
{
    char *s;

    /* Dump core if EF_DUMPCORE environment variable is defined and
       is a nonempty string; otherwise call exit(3) or _exit(2),
       depending on the value of 'useExit3'. */

    s = getenv("EF_DUMPCORE");
	
	/* if the pointer s is not NULL and its contents are not '\0' */
    if (s != NULL && *s != '\0')
		/* use the syscall abort(), one would use abort() instead
		of exit() because abort() does not call any other functions
		before abnormally terminating the process. exit() calls the
		functions registered atexit().
		Moreover, calling abort() will cause a core dump. */
        abort();
    else if (useExit3)		/* useExit3 is the Boolean input parameter
							for the terminate function() */
        exit(EXIT_FAILURE); /* use exit() stdlib function, it runs any
							function defined atexit() before terminating
							the process. */
    else
        _exit(EXIT_FAILURE); /* _exit() is like exit(3), but does not call
							 any  functions  registered  with  atexit(3)  
							 or on_exit(3). _exit() is a syscall check 
							 'man _exit' or 'man 2 exit' */
}

/* Diagnose 'errno' error by:

      * outputting a string containing the error name (if available
        in 'ename' array) corresponding to the value in 'err', along
        with the corresponding error message from strerror(), and

      * outputting the caller-supplied error message specified in
        'format' and 'ap'. */

/* define as static function, because only used inside this file!*/
static void
outputError(Boolean useErr, int err, Boolean flushStdout,
        const char *format, va_list ap)
{
#define BUF_SIZE 500
	/* initialize 3 character strings with a length of 500 */
    char buf[BUF_SIZE], userMsg[BUF_SIZE], errText[BUF_SIZE];

    vsnprintf(userMsg, BUF_SIZE, format, ap);
	/* vsnprintf() is equivalent to the function snprintf(), except that it is 
	called  with  a va_list.  
	This function does not call the va_end macro.  Because it invokes the va_arg 
	macro, the value of ap is undefined after the call. 

	snprintf writes at most BUF_SIZE bytes to the character string userMsg and ends it
	with '\0'
	*/

    if (useErr) /* useErr is a Boolean parameter for this function */
        snprintf(errText, BUF_SIZE, " [%s %s]",
                (err > 0 && err <= MAX_ENAME) ?
                ename[err] : "**UNKNOWN?? Error code**", strerror(err));
				/* if the error code err is larger than 0 and
				smaller or equal to MAX_ENAME, then use as first string 
				the error code stored in ename[err], otherwise write '**UNKNOWN?? Error code**'
				and the output of strerror(err) */
	
	/* if useErr==False then do not print Error codes */
    else
        snprintf(errText, BUF_SIZE, ":");

/* if GCC major version >= 7 then */
#if __GNUC__ >= 7
/* with pragma, compiler diagnostics can be enabled and disabled 
push (as with a stack) stores the current diagnostics settings */
#pragma GCC diagnostic push
/* disable -Wformat-truncation warning at compile time
Warn about calls to formatted input/output functions such as snprintf and vsnprintf that 
might result in output truncation. When the exact number of bytes written by a format 
directive cannot be determined at compile-time it is estimated based on heuristics that 
depend on the level argument and on optimization. While enabling optimization will 
in most cases improve the accuracy of the warning, it may also result 
in false positives. Except as noted otherwise, the option uses the same logic -Wformat-overflow.
Check for more info:
https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html#Warning-Options */
#pragma GCC diagnostic ignored "-Wformat-truncation"
#endif
    snprintf(buf, BUF_SIZE, "ERROR%s %s\n", errText, userMsg);
#if __GNUC__ >= 7
/* restore the previous diagnostics settings by using pop */
#pragma GCC diagnostic pop
#endif

    if (flushStdout)
        fflush(stdout);       /* Flush any pending stdout */
    fputs(buf, stderr);		  /* fputs() writes the string buf to the 
								 stream stderr, without its terminating 
								 null byte ('\0'). */
    fflush(stderr);           /* In case stderr is not line-buffered */
}

/* Display error message including 'errno' diagnostic, and
   return to caller */
void
errMsg(const char *format, ...)
{
    va_list argList;
    int savedErrno;

    savedErrno = errno;       /* In case we change it here */

    va_start(argList, format);
	/* print error code TRUE, and flush StdOut TRUE */
    outputError(TRUE, errno, TRUE, format, argList);
    va_end(argList);

    errno = savedErrno;
	/* errMsg does not terminate the process, it just returns
	to the caller, in comparison to errExit which executes
	terminate() -> core dump */
}

/* Display error message including 'errno' diagnostic, and
   terminate the process, if environment variable is set
   this will produce a core dump */
void
errExit(const char *format, ...)
{
    va_list argList; 		/* The include file <stdarg.h> declares 
							a type (va_list) and defines three macros
							for stepping through a list of arguments
							whose number and types are not known to
							the called function. */

    va_start(argList, format);	/* The va_start() macro must be called 
								first, and it initializes argList, which can
								be passed to va_arg() for each argument
								to be processed.  Calling va_end() signals
								that there are no further arguments,and
								causes argList to be invalidated.  Note that
								each call to va_start() must be matched
								by a call to va_end(), from within the
								same function. */

	/* print error codes TRUE, and flushStdOut TRUE */
    outputError(TRUE, errno, TRUE, format, argList);
    va_end(argList);		/* va_end call to invalidate argList as noted
							above. */

    terminate(TRUE);		/* if core dump environment value not set, 
							then exit(3) [because TRUE parameter], which
							runs functions atexit() */
}

/* Display error message including 'errno' diagnostic, and
   terminate the process by calling _exit().

   The relationship between this function and errExit() is analogous
   to that between _exit(2) and exit(3): unlike errExit(), this
   function does not flush stdout and calls _exit(2) to terminate the
   process (rather than exit(3), which would cause exit handlers to be
   invoked).

   These differences make this function especially useful in a library
   function that creates a child process that must then terminate
   because of an error: the child must terminate without flushing
   stdio buffers that were partially filled by the caller and without
   invoking exit handlers that were established by the caller. */

void
err_exit(const char *format, ...)
{
    va_list argList;

    va_start(argList, format);
    outputError(TRUE, errno, FALSE, format, argList);
    va_end(argList);

    terminate(FALSE);
}

/* The following function does the same as errExit(), but expects
   the error number in 'errnum' */

void
errExitEN(int errnum, const char *format, ...)
{
    va_list argList;

    va_start(argList, format);
    outputError(TRUE, errnum, TRUE, format, argList);
    va_end(argList);

    terminate(TRUE);
}

/* Print an error message (without an 'errno' diagnostic),
then terminate process (core dump) */
void
fatal(const char *format, ...)
{
    va_list argList;

    va_start(argList, format);
	/* Do not print 'errno' diagnostic FALSE, flush StdOut TRUE */
    outputError(FALSE, 0, TRUE, format, argList);
    va_end(argList);

    terminate(TRUE);
}

/* Print a command usage error message and terminate the process */

void
usageErr(const char *format, ...)
{
    va_list argList;

    fflush(stdout);           /* Flush any pending stdout */

    fprintf(stderr, "Usage: ");
    va_start(argList, format);
    vfprintf(stderr, format, argList);
    va_end(argList);

    fflush(stderr);           /* In case stderr is not line-buffered */
    exit(EXIT_FAILURE);
}

/* Diagnose an error in command-line arguments and
   terminate the process */

void
cmdLineErr(const char *format, ...)
{
    va_list argList;

    fflush(stdout);           /* Flush any pending stdout */

    fprintf(stderr, "Command-line usage error: ");
    va_start(argList, format);
    vfprintf(stderr, format, argList);
    va_end(argList);

    fflush(stderr);           /* In case stderr is not line-buffered */
    exit(EXIT_FAILURE);
}
/* Eduardo Rodriguez 2021 (c) (@erodrigufer) with some code taken and modified from Michael Kerrisk. Licensed under GNU AGPLv3 */
