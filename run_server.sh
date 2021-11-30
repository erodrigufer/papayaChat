#!/bin/sh
# Reference:
# 1) The filesystem:
# https://www.pathname.com/fhs/pub/fhs-2.3.html
# 2) /bin, /sbin, /usr/bin, /usr/local/bin, ...
# https://askubuntu.com/questions/308045/differences-between-bin-sbin-usr-bin-usr-sbin-usr-local-bin-usr-local

SYSTEM_USER=papayachat

SERVER_BIN_PATH=./bin/concurrent_server.bin

DAEMON_EXECUTABLE_NAME=papayachatd

TERM_BIN_PATH=./bin/termHandlerAsyncSafe.bin
TERM_EXECUTABLE_NAME=$(basename ${TERM_BIN_PATH})

# Check:
# https://askubuntu.com/questions/308045/differences-between-bin-sbin-usr-bin-usr-sbin-usr-local-bin-usr-local
# Path to copy executable (binary) of daemon
INSTALLATION_PATH=/usr/local/bin/papayachat/

INSTALLATION_FILE=${INSTALLATION_PATH}${DAEMON_EXECUTABLE_NAME}
INSTALLATION_TERM=${INSTALLATION_PATH}${TERM_EXECUTABLE_NAME}

CHATLOG_PATH=/var/lib/papayachat/
CHATLOG_FILENAME=papayachat.chat

CHATLOG_FILE=${CHATLOG_PATH}${CHATLOG_FILENAME}

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

uninstall(){
	echo "Uninstalling ${DAEMON_EXECUTABLE_NAME}..."
	sudo rm -rf ${INSTALLATION_PATH}
	sudo rm -rf ${CHATLOG_PATH}
	echo "All files removed. Done!"
	exit 0

}

# execute this function if installation fails,
# functions removes all installation files 
defer_installation(){
	sudo rm -rf ${INSTALLATION_PATH}
	sudo rm -rf ${CHATLOG_PATH}

	exit -1
}

# check if daemon is already installed in the system,
# otherwise install binary on installation path
# and create file to store chat log
install_daemon(){
	
	# check if daemon is already istalled
	[ -f ${INSTALLATION_FILE} ] && return 0

	echo "Compilling and testing..."
	# if it is not installed, then compile and test
	make clean	
	make test || { echo "[ERROR] Server compilation/test failed!"; exit -1 ; }

	make server
	
	echo "Installing daemon at ${INSTALLATION_FILE}..."
	sudo mkdir -p ${INSTALLATION_PATH} || { echo "[ERROR] directory creation at ${INSTALLATION_PATH}"; exit -1 ; }
	
	# Copy daemon to installation path
	sudo cp ${SERVER_BIN_PATH} ${INSTALLATION_FILE} || { echo "[ERROR] Binary installation failed!"; exit -1 ; }

	# Copy other necessary binary files to installation path
	sudo cp ${TERM_BIN_PATH} ${INSTALLATION_TERM} || { echo "[ERROR] Binary installation failed!"; exit -1 ; }

	# Change file ownership to root, only root can modify executable
	# root and papayachat con execute file
	# If any of this commands fails, remove binary (security risk!)
	sudo chown root:${SYSTEM_USER} ${INSTALLATION_FILE} || { echo "[ERROR] chown failed!"; defer_installation ; } 

	sudo chown root:${SYSTEM_USER} ${INSTALLATION_TERM} || { echo "[ERROR] chown failed!"; defer_installation ; } 

	# Change file permission, so that only root and papayachat con execute
	sudo chmod 750 ${INSTALLATION_FILE} || { echo "[ERROR] chmod failed!"; defer_installation ; } 

	sudo chmod 750 ${INSTALLATION_TERM} || { echo "[ERROR] chmod failed!"; defer_installation ; } 
	
	echo "daemon ${DAEMON_EXECUTABLE_NAME} installed properly!"
		
}

create_chat_log(){

	# check if chatlog already exists and is a regular file
	[ -f ${CHATLOG_FILE} ] && return 0
	
	echo "Creating chat log file at ${CHATLOG_FILE}..."

	sudo mkdir -p ${CHATLOG_PATH} || { echo "[ERROR] directory creation at ${CHATLOG_PATH}"; exit -1 ; }
	
	sudo touch ${CHATLOG_FILE} || { echo "[ERROR] file creation failed!"; exit -1 ; }

	# Change file ownership
	sudo chown root:${SYSTEM_USER} ${CHATLOG_FILE} || { echo "[ERROR] chatlog chown failed!"; defer_installation ; } 
	
	# only read/write permissions for root and group
	sudo chmod 660 ${CHATLOG_FILE} || { echo "[ERROR] chatlog chmod failed!"; defer_installation ; } 

}

main_installation(){
	check_distribution

	create_system_user

	# copy daemon executable to installation path
	install_daemon

	# create chat log
	create_chat_log

	exit 0
	
}

run_server(){

	# check if daemon is already istalled, if so, run papayachat as system user
	[ -f ${INSTALLATION_FILE} ] && sudo -u ${SYSTEM_USER} ${INSTALLATION_FILE} && echo "Executing papayachatd"

	exit 0

}

# -u flag, run uninstall
[ "$1" = '-u' ] && uninstall 
[ "$1" = '--uninstall' ] && uninstall 
[ "$1" = '-r' ] && run_server
main_installation
