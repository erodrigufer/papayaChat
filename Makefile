# Makefile 

# Objects and executable for daemonLogger
OBJECTS1 = daemonCreation.o daemonLogger.o
EXECUTABLE1 = daemonLogger

# Objects and executable for concurrent_server
OBJECTS2 = concurrent_server.o error_handling.o inet_sockets.o daemonCreation.o configure_syslog.o termHandlerAsyncSafe
EXECUTABLE2 = concurrent_server 

OBJECTS = $(OBJECTS1) $(OBJECTS2)
EXECUTABLES = $(EXECUTABLE1) $(EXECUTABLE2)

all : $(EXECUTABLES)

# Link the object files to the compiled program
# using all Warning
# daemonLogger
$(EXECUTABLE1) : $(OBJECTS1)
	cc -Wall -o $(EXECUTABLE1) $(OBJECTS1)

# concurrent_server
$(EXECUTABLE2) : $(OBJECTS2)
	cc -Wall -o $(EXECUTABLE2) $(OBJECTS2)

termHandlerAsyncSafe : termHandlerAsyncSafe.o
	cc -Wall -o termHandlerAsyncSafe termHandlerAsyncSafe.o

termHandlerAsyncSafe.o : basics.h
# Implicit rules, daemonCreation.c is missing
# cc -c daemonCreation.c is also not required
daemonCreation.o : basics.h daemonCreation.h

daemonLogger.o : basics.h daemonCreation.h

concurrent_server.o : inet_sockets.o inet_sockets.h basics.h daemonCreation.o daemonCreation.h error_handling.o configure_syslog.o

error_handling.o : error_handling.h basics.h error_names.c.inc

error_names.c.inc :
	sh Build_error_names.sh > error_names.c.inc
	# 1>&2 means redirect stdout to stderr
	echo 1>&2 "error_names.c.inc built"

inet_sockets.o : inet_sockets.h basics.h

configure_syslog.o : 

# Remove object files and linked program
.PHONY : clean
clean :
	rm $(EXECUTABLES) $(OBJECTS) error_names.c.inc



# Eduardo Rodriguez 2021 (c) @erodrigufer. Licensed under GNU AGPLv3
