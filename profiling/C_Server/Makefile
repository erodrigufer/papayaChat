# Makefile 

# Define compiler
CC = gcc

# Enable most warnings, and make warnings behave as errors
CC_FLAGS = -Wall -Werror

# Objects and executable for concurrent_server
OBJECTS_SERVER = C_Server.o error_handling.o inet_sockets.o signalHandling.o 
EXECUTABLE_SERVER = ./server.bin

# Compile only server
.PHONY : server
server : $(EXECUTABLE_SERVER)

# Link the object files to the compiled program
# using all Warning

# concurrent_server
$(EXECUTABLE_SERVER) : $(OBJECTS_SERVER) 
	$(CC) $(CC_FLAGS) -o $(EXECUTABLE_SERVER) $(OBJECTS_SERVER)

C_Server.o : inet_sockets.o inet_sockets.h basics.h error_handling.o signalHandling.o 

error_handling.o : error_handling.h basics.h error_names.c.inc

error_names.c.inc :
	sh Build_error_names.sh > error_names.c.inc
	@# 1>&2 means redirect stdout to stderr
	echo 1>&2 "error_names.c.inc built"

inet_sockets.o : inet_sockets.h basics.h

signalHandling.o :

.PHONY : run
run : $(EXECUTABLE_SERVER)
	./$(EXECUTABLE_SERVER)
	

# Remove object files, executables and error names file (system dependant)
.PHONY : clean
clean :
	@rm -f *.bin *.o error_names.c.inc 

# Eduardo Rodriguez 2021 (c) @erodrigufer. Licensed under GNU AGPLv3
