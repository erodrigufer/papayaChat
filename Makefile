# Makefile for 'daemonLog' 

OBJECTS = daemonCreation.o main.o

PROGRAM_NAME = daemonLog

$(PROGRAM_NAME) : $(OBJECTS)
	cc -o $(PROGRAM_NAME) $(OBJECTS)

# Implicit rules, daemonCreation.c is missing
# cc -c daemonCreation.c is also not required
daemonCreation.o : basics.h daemonCreation.h

main.o : basics.h daemonCreation.h

# Remove object files and linked program
.PHONY : clean
clean :
	rm $(PROGRAM_NAME) $(OBJECTS)
