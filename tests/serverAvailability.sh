#!/bin/sh
# Change path so that relative path works when running script from makefile
cd $(dirname $0)

SYSTEM_USER=papayachat

# Daemon executable
EXECUTABLE=./concurrent_server_test.bin
TERM_FILE=./termHandlerAsyncSafe.bin

EXECUTABLE_FILES=${EXECUTABLE} ${TERM_FILE}

COLOR_GREEN='\e[0;32m'
NO_COLOR='\033[0m'
COLOR_RED='\033[0;31m'

compile_parser(){
	gcc -o parser.bin test_configParser.c ../configParser.o
}

parse_port(){
	compile_parser 

	./parser.bin ../etc/server.config PORT
}

PORT=$(parse_port)
# sleep after starting daemon, because daemon needs some time to be up and running
SLEEP_TIME=2 # in seconds

# Remove executables and chatlog after test
defer(){
	sudo rm -f *.bin *.chat
}

echo "[test] ...Starting server availability test..."
echo "* Checking dependencies..."
# ss (instead of netstat)
which ss > /dev/null || { echo "ss is missing"; printf "[${COLOR_RED}MISSING DEPENDENCY${NO_COLOR}] Server availability test failed!\n"; exit -1; }
# Check if PORT is already in use
# ss -a (listening and active ports) -t (TCP connections)
ss -at | grep ${PORT} && { printf "[${COLOR_RED}FAILED${NO_COLOR}] Port ${PORT} already in use. A server instance is probably already running. Exit test!\n"; exit -1; }

sudo touch ./chat_log.chat
sudo chown root:${SYSTEM_USER} ./chat_log.chat
sudo chmod 660 ./chat_log.chat

# netcat -zv Verbose output -z check for connection
sudo -u ${SYSTEM_USER} ${EXECUTABLE} && { echo "* Starting server..."; sleep ${SLEEP_TIME}; echo "* Server daemon is now running..."; ss -at | grep ${PORT}; } && netcat -zv localhost ${PORT} || { printf "[${COLOR_RED}FAILED${NO_COLOR}] Server availability test failed!\n"; defer; exit -1; }

sudo kill $(pidof ${EXECUTABLE}) && echo "* Killed daemon after test..."
ss -at | grep ${PORT}
printf "[${COLOR_GREEN}SUCCESS${NO_COLOR}] Server availability test passed!\n"

# Remove executables and other files created for test
defer

exit 0

# Eduardo Rodriguez 2021 (c) (@erodrigufer). Licensed under GNU AGPLv3
