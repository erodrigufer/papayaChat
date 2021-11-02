# Makefile 

# Enable most warnings, and make warnings behave as errors
CC_FLAGS = -Wall -Werror

OBJECTS_CLIENT = client.o error_handling.o inet_sockets.o
EXECUTABLE_CLIENT = papayaChat_client

OBJECTS_FRONTEND = frontEnd.o error_handling.o inet_sockets.o
EXECUTABLE_FRONTEND = frontEnd

# Objects and executable for daemonLogger
OBJECTS_DAEMON_LOGGER = daemonCreation.o daemonLogger.o
EXECUTABLE_DAEMON_LOGGER = daemonLogger

# Objects and executable for concurrent_server
OBJECTS_SERVER = concurrent_server.o error_handling.o inet_sockets.o daemonCreation.o configure_syslog.o 
EXECUTABLE_SERVER = concurrent_server 

OBJECTS = $(OBJECTS_DAEMON_LOGGER) $(OBJECTS_SERVER) termHandlerAsyncSafe.o $(OBJECTS_CLIENT) $(OBJECTS_FRONTEND)
EXECUTABLES = $(EXECUTABLE_DAEMON_LOGGER) $(EXECUTABLE_SERVER) termHandlerAsyncSafe $(EXECUTABLE_CLIENT) $(EXECUTABLE_FRONTEND)

all : $(EXECUTABLES)

# Link the object files to the compiled program
# using all Warning
# daemonLogger
$(EXECUTABLE_DAEMON_LOGGER) : $(OBJECTS_DAEMON_LOGGER)
	cc $(CC_FLAGS) -o $(EXECUTABLE_DAEMON_LOGGER) $(OBJECTS_DAEMON_LOGGER)

# concurrent_server
$(EXECUTABLE_SERVER) : $(OBJECTS_SERVER)
	cc $(CC_FLAGS) -o $(EXECUTABLE_SERVER) $(OBJECTS_SERVER)

#termHandlerAsyncSafe
termHandlerAsyncSafe : termHandlerAsyncSafe.o configure_syslog.o
	cc $(CC_FLAGS) -o termHandlerAsyncSafe termHandlerAsyncSafe.o configure_syslog.o

termHandlerAsyncSafe.o : basics.h configure_syslog.o configure_syslog.h
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

# run front-end executable
.PHONY : run 
run : $(EXECUTABLE_FRONTEND)
	./$(EXECUTABLE_FRONTEND)

frontEnd.o : basics.h error_handling.o inet_sockets.o

# frontEnd with ncurses
$(EXECUTABLE_FRONTEND) : $(OBJECTS_FRONTEND)
	cc $(CC_FLAGS) -o $(EXECUTABLE_FRONTEND) $(OBJECTS_FRONTEND) -lncurses

.PHONY : client
client: $(EXECUTABLE_CLIENT)

$(EXECUTABLE_CLIENT) : $(OBJECTS_CLIENT)
	cc $(CC_FLAGS) -o $(EXECUTABLE_CLIENT) $(OBJECTS_CLIENT)

client.o : inet_sockets.h basics.h error_handling.o inet_sockets.o

# Remove object files, executables and error names file (system dependant)
.PHONY : clean
clean :
	rm $(EXECUTABLES) *.o error_names.c.inc 

# Eduardo Rodriguez 2021 (c) @erodrigufer. Licensed under GNU AGPLv3
