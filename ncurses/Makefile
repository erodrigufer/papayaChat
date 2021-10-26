CC_FLAGS = -Wall -Werror

EXECUTABLE = papayaChat_client

papayaChat : frontEnd.c
	gcc $(CC_FLAGS) -o $(EXECUTABLE) frontEnd.c -lncurses

clean:
	rm $(EXECUTABLE)

