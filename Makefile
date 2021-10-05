# Makefile for 'daemonLog' 

OBJECTS = daemonCreation.o main.o

PROGRAM_NAME = daemonLog

# Link the object files to the compiled program
# using all Warning
$(PROGRAM_NAME) : $(OBJECTS)
	cc -Wall -o $(PROGRAM_NAME) $(OBJECTS)

# Implicit rules, daemonCreation.c is missing
# cc -c daemonCreation.c is also not required
daemonCreation.o : basics.h daemonCreation.h

main.o : basics.h daemonCreation.h

# Remove object files and linked program
.PHONY : clean
clean :
	rm $(PROGRAM_NAME) $(OBJECTS)
