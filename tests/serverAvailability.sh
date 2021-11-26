#!/bin/sh

COLOR_GREEN='\e[0;32m'
NO_COLOR='\033[0m'
COLOR_RED='\033[0;31m'

echo "[test] Test server availability..."
# Install dependencies
# ss (instead of netstat)
echo "Check dependencies..."
which ss || { echo "ss is missing"; printf "[${COLOR_RED}MISSING DEPENDENCY${NO_COLOR}] Server availability test failed!\n"; exit -1; }

PORT=51000
# sleep after starting daemon, because daemon needs some time to be up and running
SLEEP_TIME=2 # in seconds

# netcat -zv Verbose output -z check for connection
../bin/concurrent_server.bin && { echo "Starting server..."; sleep ${SLEEP_TIME}; echo "Server daemon is now running..."; ss -at | grep ${PORT}; } && netcat -zv localhost ${PORT} || { printf "[${COLOR_RED}FAILED${NO_COLOR}] Server availability test failed!\n"; exit -1; }

kill $(pidof concurrent_server.bin) && echo "Killed daemon..."
# ss -a (listening and active ports) -t (TCP connections)
ss -at | grep ${PORT}
printf "[${COLOR_GREEN}SUCCESS${NO_COLOR}] Server availability test passed!\n"
exit 0

