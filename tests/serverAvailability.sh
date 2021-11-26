#!/bin/sh

# Install dependencies
# ss (instead of netstat)
echo "Check dependencies..."
which -s ss || { echo "ss is missing"; sudo apt-get install ss; }

PORT=51000
# sleep after starting daemon, because daemon needs some time to be up and running
SLEEP_TIME=3 # in seconds

# netcat -zv Verbose output -z check for connection
../bin/concurrent_server.bin && { echo "Starting server..."; sleep ${SLEEP_TIME}; echo "Server daemon running."; ss -at | grep ${PORT}; } && netcat -zv localhost ${PORT} || { echo "Server availability failed!"; exit -1; }

kill $(pidof concurrent_server.bin) && echo "Killed daemon"
# ss -a (listening and active ports) -t (TCP connections)
ss -at | grep ${PORT}
echo "Connection test passed!"
exit 0

