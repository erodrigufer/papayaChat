#!/bin/sh
# Change path so that relative path works when running script from makefile
cd $(dirname $0)

# Daemon executable
EXECUTABLE=./concurrent_server.bin

COLOR_GREEN='\e[0;32m'
NO_COLOR='\033[0m'
COLOR_RED='\033[0;31m'

PORT=51000
# sleep after starting daemon, because daemon needs some time to be up and running
SLEEP_TIME=2 # in seconds

echo "[test] Test server availability..."
echo "Check dependencies..."
# ss (instead of netstat)
which ss || { echo "ss is missing"; printf "[${COLOR_RED}MISSING DEPENDENCY${NO_COLOR}] Server availability test failed!\n"; exit -1; }
# Check if PORT is already in use
ss -at | grep ${PORT} && { printf "[${COLOR_RED}FAILED${NO_COLOR}] Port ${PORT} already in use. A server instance is probably already running. Exit test!\n"; exit -1; }

cd ../bin
# netcat -zv Verbose output -z check for connection
${EXECUTABLE} && { echo "Starting server..."; sleep ${SLEEP_TIME}; echo "Server daemon is now running..."; ss -at | grep ${PORT}; } && netcat -zv localhost ${PORT} || { printf "[${COLOR_RED}FAILED${NO_COLOR}] Server availability test failed!\n"; exit -1; }

kill $(pidof ${EXECUTABLE}) && echo "Killed daemon..."
# ss -a (listening and active ports) -t (TCP connections)
ss -at | grep ${PORT}
printf "[${COLOR_GREEN}SUCCESS${NO_COLOR}] Server availability test passed!\n"
exit 0

# Eduardo Rodriguez 2021 (c) (@erodrigufer). Licensed under GNU AGPLv3
