EXECUTABLE_SERVER = ./server.bin
FILES_SERVER = server.go

.PHONY : server
server : $(EXECUTABLE_SERVER)

$(EXECUTABLE_SERVER) : $(FILES_SERVER)
	go build -o $(EXECUTABLE_SERVER) $(FILES_SERVER)

.PHONY : run
run : $(EXECUTABLE_SERVER)
	./$(EXECUTABLE_SERVER)
	

.PHONY : clean
clean :
	@rm -f *.bin 

# Eduardo Rodriguez 2021 (c) @erodrigufer. Licensed under GNU AGPLv3
