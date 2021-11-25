# Makefile 

# Enable most warnings, and make warnings behave as errors
CC_FLAGS = -Wall -Werror

OBJECTS_FRONTEND = frontEnd.o error_handling.o inet_sockets.o signalHandling.o handleMessages.o
EXECUTABLE_FRONTEND = ./bin/frontEnd.bin

# Objects and executable for concurrent_server
OBJECTS_SERVER = concurrent_server.o error_handling.o inet_sockets.o daemonCreation.o configure_syslog.o file_locking.o signalHandling.o clientRequest.o
EXECUTABLE_SERVER = ./bin/concurrent_server.bin

EXECUTABLE_TERMHANDLER = ./bin/termHandlerAsyncSafe.bin

OBJECTS = $(OBJECTS_SERVER) termHandlerAsyncSafe.o $(OBJECTS_FRONTEND)
EXECUTABLES = $(EXECUTABLE_SERVER) $(EXECUTABLE_TERMHANDLER) $(EXECUTABLE_FRONTEND)

.PHONY : all

all : $(EXECUTABLES)
# TODO: I still have not been able to implement the pre-requisite of creating the bin directory
# Create bin directory if it does not exist
# -p is needed to exit successfully even if directory exists
#	mkdir -p ./bin/

# Compile only server
.PHONY : server
server : $(EXECUTABLE_SERVER)

# Link the object files to the compiled program
# using all Warning

# concurrent_server
$(EXECUTABLE_SERVER) : $(OBJECTS_SERVER) $(EXECUTABLE_TERMHANDLER)
	cc $(CC_FLAGS) -o $(EXECUTABLE_SERVER) $(OBJECTS_SERVER)

#termHandlerAsyncSafe
$(EXECUTABLE_TERMHANDLER) : termHandlerAsyncSafe.o configure_syslog.o
	cc $(CC_FLAGS) -o $(EXECUTABLE_TERMHANDLER) termHandlerAsyncSafe.o configure_syslog.o

termHandlerAsyncSafe.o : basics.h configure_syslog.o configure_syslog.h
# Implicit rules, daemonCreation.c is missing
# cc -c daemonCreation.c is also not required
daemonCreation.o : basics.h daemonCreation.h

concurrent_server.o : inet_sockets.o inet_sockets.h basics.h daemonCreation.o daemonCreation.h error_handling.o configure_syslog.o file_locking.o signalHandling.o clientRequest.o

error_handling.o : error_handling.h basics.h error_names.c.inc

clientRequest.o : file_locking.o

error_names.c.inc :
	sh Build_error_names.sh > error_names.c.inc
	# 1>&2 means redirect stdout to stderr
	echo 1>&2 "error_names.c.inc built"

inet_sockets.o : inet_sockets.h basics.h

configure_syslog.o :

file_locking.o : CONFIG.h

signalHandling.o :

handleMessages.o :

# run front-end executable
.PHONY : run 
run : $(EXECUTABLE_FRONTEND)
	$(EXECUTABLE_FRONTEND)

.PHONY : client
client: $(EXECUTABLE_FRONTEND)

frontEnd.o : basics.h error_handling.o inet_sockets.o signalHandling.o handleMessages.o CONFIG.h 

# frontEnd with ncurses
# link to ncurses library with '-lncurses'
$(EXECUTABLE_FRONTEND) : $(OBJECTS_FRONTEND)
	cc $(CC_FLAGS) -o $(EXECUTABLE_FRONTEND) $(OBJECTS_FRONTEND) -lncurses

# Remove object files, executables and error names file (system dependant)
.PHONY : clean
clean :
	rm $(EXECUTABLES) *.o error_names.c.inc 

# Eduardo Rodriguez 2021 (c) @erodrigufer. Licensed under GNU AGPLv3
