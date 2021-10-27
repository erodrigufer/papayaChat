CC_FLAGS = -Wall -Werror

EXECUTABLE = papayaChat_client

OBJECTS = frontEnd.o

$(EXECUTABLE) : $(OBJECTS)
	gcc $(CC_FLAGS) -o $(EXECUTABLE) $(OBJECTS) -lncurses

frontEnd.o :

.PHONY : run
run: $(EXECUTABLE)
	./$(EXECUTABLE)

.PHONY : clean
clean:
	rm $(EXECUTABLE) $(OBJECTS)

