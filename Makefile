# Makefile 

# Objects and executable for daemonLogger
OBJECTS1 = daemonCreation.o daemonLogger.o
EXECUTABLE1 = daemonLogger

# Objects and executable for concurrent_server
OBJECTS2 = concurrent_server.o
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

# Implicit rules, daemonCreation.c is missing
# cc -c daemonCreation.c is also not required
daemonCreation.o : basics.h daemonCreation.h

daemonLogger.o : basics.h daemonCreation.h

concurrent_server.o : inet_sockets.o inet_sockets.h basics.h daemonCreation.o daemonCreation.h

inet_sockets.o : inet_sockets.h basics.h

# Remove object files and linked program
.PHONY : clean
clean :
	rm $(EXECUTABLES) $(OBJECTS)



# Eduardo Rodriguez 2021 (c) @erodrigufer. Licensed under GNU AGPLv3
