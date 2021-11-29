#!/bin/sh
# Reference:
# The filesystem:
# https://www.pathname.com/fhs/pub/fhs-2.3.html

SYSTEM_USER=papayachat

SERVER_BIN=concurrent_server.bin

# Some commands are debian-based (even probably BSD compliant)
# so if the distro is not debian-based exit
check_distribution(){
	# print Linux_Standard_Base release ID
	# use grep in egrep mode (-e), being 
	# case-insensitive (-i) and check for either 
	# ubuntu or debian
	lsb_release -i | grep -i -E "(ubuntu|debian)" || { echo "[ERROR] System must be debian-based to run daemon."; exit -1 ; }

	# About egrep: with egrep it is easier to write the OR
	# logic ( | ) otherwise all these characters must be escaped
	# with the normal regex grammar of grep
}

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

main(){
	check_distribution
	 
	#TODO: redirect output of id to make it silent later
	# if the system_user does not exist, it is created
	id ${SYSTEM_USER} || create_system_user

	# compile and test the server
	make test || exit -1

	# Change working directory to ./bin
	cd ./bin

	echo "sudo -u ${SYSTEM_USER} ./${SERVER_BIN}"
	#./${SERVER_BIN}

}


main
