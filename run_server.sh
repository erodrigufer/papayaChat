#!/bin/sh

SYSTEM_USER=papayachat

SERVER_BIN=concurrent_server.bin

create_system_user(){

	echo "Creating system user..."
	# add a new system user, without login, without a home directory
	# and with its own group
	sudo adduser --system --no-create-home --group ${SYSTEM_USER} || exit -1

	# Some remarks:
	# - After creating the user, no home folder for this user should be found on
	# /home
	# - running `sudo -u papayachat whoami` should display `papayachat`
	# - `grep papayachat /etc/passwd /etc/shadow` should show info about the user

}

#TODO: redirect output of id to make it silent later
id ${SYSTEM_USER} || create_system_user

# compile and test the server
# make test || exit -1

# Change working directory to ./bin
cd ./bin

echo "sudo -u ${SYSTEM_USER} ./${SERVER_BIN}"
#./${SERVER_BIN}

