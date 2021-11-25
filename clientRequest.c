#include "clientRequest.h"

/* Handle a client request: copy socket input back to socket,
good coding practices, function is static, because it should only
be available in this local script */
void
handleRequest(int client_fd)
{
    char buf[BUF_SIZE];
    ssize_t numRead;

	/* define greetingMessage string, the compiler allocates enough memory for the string */
	const char greetingMessage [] = "\
\n\
--------------------------------------------------------------\n\
                  ...PapayaChat Service...\n\n\
PapayaChat is licensed under GNU AGPLv3.\n\
The code is hosted at: www.github.com/erodrigufer/papayaChat\n\
--------------------------------------------------------------\n\
\n\
";

	/* the size of the string is calculated statically at compile time,
	check 'Effective C' page 133 */
	size_t greetingSize = sizeof greetingMessage;
	
	/* the write() call should write exactly greetingSize bytes, otherwise
	it has failed */
	if(write(client_fd,greetingMessage,greetingSize)!=greetingSize){
		syslog(LOG_ERR, "write() failed: %s", strerror(errno));
		_exit(EXIT_FAILURE);
	}
    
	while ((numRead = read(client_fd, buf, BUF_SIZE)) > 0) {
        if (write(client_fd, buf, numRead) != numRead) {
            syslog(LOG_ERR, "write() failed: %s", strerror(errno));
            _exit(EXIT_FAILURE);
        }
		/* add debug syslog to see amount of bytes received from client */
		syslog(LOG_DEBUG, "%ld Bytes received from client.", numRead);
    }

    if (numRead == -1) {
        syslog(LOG_ERR, "Error from read(): %s", strerror(errno));
        _exit(EXIT_FAILURE);
    }

	/* if the client closes its connection, the previous read() syscall will get an
	EOF, and it will return 0, in that case, the while-loop ends, and there is no 
	syslog error appended to the log, since read() did not return an error */
}

