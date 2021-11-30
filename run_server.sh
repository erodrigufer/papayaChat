#!/bin/sh
# Reference:
# 1) The filesystem:
# https://www.pathname.com/fhs/pub/fhs-2.3.html
# 2) /bin, /sbin, /usr/bin, /usr/local/bin, ...
# https://askubuntu.com/questions/308045/differences-between-bin-sbin-usr-bin-usr-sbin-usr-local-bin-usr-local

SYSTEM_USER=papayachat

SERVER_BIN_PATH=./bin/concurrent_server.bin

DAEMON_EXECUTABLE_NAME=papayachatd

# Check:
# https://askubuntu.com/questions/308045/differences-between-bin-sbin-usr-bin-usr-sbin-usr-local-bin-usr-local
# Path to copy executable (binary) of daemon
INSTALLATION_PATH=/usr/local/bin/

INSTALLATION_FILE=${INSTALLATION_PATH}${DAEMON_EXECUTABLE_NAME}

CHATLOG_PATH=/var/local
CHATLOG_FILENAME=papayachat.chat

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
	 
	#TODO: redirect output of id to make it silent later
	
	# if the system_user does not exist, it is created
	id ${SYSTEM_USER} && return 0

	echo "Creating system user (UID=papayachat) ..."
	# add a new system user, without login, without a home directory
	# and with its own group
	sudo adduser --system --no-create-home --group ${SYSTEM_USER} || exit -1

	# Some remarks:
	# - After creating the user, no home folder for this user should be found on
	# /home
	# - running `sudo -u papayachat whoami` should display `papayachat`
	# - `grep papayachat /etc/passwd /etc/shadow` should show info about the user

}

# check if daemon is already installed in the system,
# otherwise install binary on installation path
# and create file to store chat log
install_daemon(){
	
	# check if daemon is already istalled
	which ${DAEMON_EXECUTABLE_NAME} && return 0

	echo "Installing daemon at ${INSTALLATION_PATH}..."

	# if it is not installed, then compile and test
	make clean	
	make test || { echo "[ERROR] Server compilation/test failed!"; exit -1 ; }

	# Copy daemon to installation path
	sudo cp ${SERVER_BIN_PATH} ${INSTALLATION_FILE} || { echo "[ERROR] Binary installation failed!"; exit -1 ; }

	# Change file ownership to root, only root can modify executable
	# root and papayachat con execute file
	# If any of this commands fails, remove binary (security risk!)
	sudo chown root:papayachat ${INSTALLATION_FILE} || { echo "[ERROR] chown failed!"; sudo rm -f ${INSTALLATION_FILE}; exit -1 ; } 

	# Change file permission, so that only root and papayachat con execute
	sudo chmod 750 ${INSTALLATION_FILE} || { echo "[ERROR] chmod failed!"; sudo rm -f ${INSTALLATION_FILE}; exit -1 ; } 
	
	echo "daemon ${DAEMON_EXECUTABLE_NAME} installed properly!"
		
}

main_installation(){
	check_distribution

	create_system_user

	# copy daemon executable to installation path
	install_daemon

	# create chat log
	
	echo "sudo -u ${SYSTEM_USER} ./${SERVER_BIN}"
	#./${SERVER_BIN}

}


main_installation
