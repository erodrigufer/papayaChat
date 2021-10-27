CC_FLAGS = -Wall -Werror

EXECUTABLE = papayaChat_client

$(EXECUTABLE) : frontEnd.c
	gcc $(CC_FLAGS) -o $(EXECUTABLE) frontEnd.c -lncurses

.PHONY : run
run: 
	./$(EXECUTABLE)

.PHONY : clean
clean:
	rm $(EXECUTABLE)

