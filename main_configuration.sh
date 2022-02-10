#!/bin/sh
# Reference:
# 1) The filesystem:
# https://www.pathname.com/fhs/pub/fhs-2.3.html
# 2) /bin, /sbin, /usr/bin, /usr/local/bin, ...
# https://askubuntu.com/questions/308045/differences-between-bin-sbin-usr-bin-usr-sbin-usr-local-bin-usr-local

# Definition of colours
COLOR_GREEN='\e[0;32m'
NO_COLOR='\033[0m'
COLOR_RED='\033[0;31m'

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

# ----------- Client's variables---------------------------------------
CLIENT_INSTALLATION_PATH=${HOME}/bin
CLIENT_INSTALLATION_FILE=${CLIENT_INSTALLATION_PATH}/papayachat
# ----------- Config files --------------------------------------------
CLIENT_CONFIG_FILE_PATH=~/.papayachat/ 
SERVER_CONFIG_FILE_PATH=/etc/papayachat/
CLIENT_CONFIG_NAME=client.config
SERVER_CONFIG_NAME=server.config
SERVER_DEFAULT_KEY=key # name of server key file

REPO_CONFIG_PATH=./etc/

# Use these as pathnames for config files
CLIENT_CONFIG=${CLIENT_CONFIG_FILE_PATH}${CLIENT_CONFIG_NAME}
SERVER_CONFIG=${SERVER_CONFIG_FILE_PATH}${SERVER_CONFIG_NAME}
ETC_KEY_PATH=${SERVER_CONFIG_FILE_PATH}${SERVER_DEFAULT_KEY}
CLIENT_KEY_PATH=${CLIENT_CONFIG_FILE_PATH}${SERVER_DEFAULT_KEY}

# Paths inside repo for default config files
CLIENT_REPO_CONFIG=${REPO_CONFIG_PATH}${CLIENT_CONFIG_NAME}
SERVER_REPO_CONFIG=${REPO_CONFIG_PATH}${SERVER_CONFIG_NAME}
REPO_KEY_FILE=${REPO_CONFIG_PATH}${SERVER_DEFAULT_KEY} # default key

# ---------------------------------------------------------------------

# Some commands are debian-based (even probably BSD compliant)
# so if the distro is not debian-based exit
check_distribution(){
	# print Linux_Standard_Base release ID
	# use grep in egrep mode (-e), being 
	# case-insensitive (-i) and check for either 
	# ubuntu or debian
	echo "* Checking system distribution..."
	lsb_release -i | grep -i -E "(ubuntu|debian|kali)" || { printf "[${COLOR_RED}ERROR${NO_COLOR}] System must be debian-based to run daemon.\n"; exit -1 ; }

	# About egrep: with egrep it is easier to write the OR
	# logic ( | ) otherwise all these characters must be escaped
	# with the normal regex grammar of grep
}

create_system_user(){
	
	echo "* Checking existance of correct system user for daemon..."
	# if the system_user does not exist, it is created
	id ${SYSTEM_USER} > /dev/null && return 0

	echo "* Creating system user (UID=papayachat) ..."
	# add a new system user, without login, without a home directory
	# and with its own group
	sudo adduser --system --no-create-home --group ${SYSTEM_USER} || exit -1

	# Some remarks:
	# - After creating the user, no home folder for this user should be found on
	# /home
	# - running `sudo -u papayachat whoami` should display `papayachat`
	# - `grep papayachat /etc/passwd /etc/shadow` should show info about the user

}

# Kill a running daemon of papayachatd
kill_daemon(){

	# ps -ef will show all processes with its PIDs in $2 and its PPIDs in $3
	# so if the PPID of one of the papayachat instances equals 1, we know that
	# it most be the daemon, since its parent is init
	# in that case, print the second column $2, and use that value to kill the daemon
	# in case that the daemon is not running, supress the error message of kill by redirecting to
	# /dev/null
	sudo kill $(ps -ef | grep papayachat | awk '{ if ($3 == 1) print $2 }') 2>/dev/null && { printf "[${COLOR_GREEN}DOWN${NO_COLOR}] papayachatd daemon killed!\n"; return 0; }

	printf "[${COLOR_RED}WARNING${NO_COLOR}] papayachatd daemon was NOT killed! daemon was NOT currently running in the system!\n"
}

# Remove only client's files
uninstall_client(){
	
	echo "* Removing client's config files..."
	rm -rf ${CLIENT_CONFIG_FILE_PATH} || { printf "[${COLOR_RED}ERROR${NO_COLOR}] client config files could not be removed!\n"; exit -1 ; }
	echo "* Removing client's executable in shell's path..."
	rm -rf ${CLIENT_INSTALLATION_FILE} || { printf "[${COLOR_RED}ERROR${NO_COLOR}] client executable could not be removed!\n"; exit -1 ; }
	echo "All files removed. Done!"
	return 0

}

# Remove only server's files
uninstall_server(){
	kill_daemon
	echo "* Un-installing ${DAEMON_EXECUTABLE_NAME}..."
	sudo rm -rf ${INSTALLATION_PATH} || { printf "[${COLOR_RED}ERROR${NO_COLOR}] daemon executable could not be removed!\n"; exit -1 ; }
	echo "* Removing chat log file..."
	sudo rm -rf ${CHATLOG_PATH} || { printf "[${COLOR_RED}ERROR${NO_COLOR}] chatlog file could not be removed!\n"; exit -1 ; }
	echo "* Removing server's config files..."
	sudo rm -rf ${SERVER_CONFIG_FILE_PATH} || { printf "[${COLOR_RED}ERROR${NO_COLOR}] server config files could not be removed!\n"; exit -1 ; }
	echo "All files removed. Done!"
	return 0

}
# Remove both client's and server's files
uninstall(){

	[ "$1" == '--client' ] && uninstall_client  
	[ "$1" == '--server' ] && uninstall_server 

	return 0
	
}

# execute this function if installation fails,
# functions removes all installation files 
defer_installation(){
	sudo rm -rf ${INSTALLATION_PATH}
	sudo rm -rf ${CHATLOG_PATH}
	sudo rm -rf ${CLIENT_CONFIG_FILE_PATH}
	sudo rm -rf ${SERVER_CONFIG_FILE_PATH}

	exit -1
}

create_server_config_files(){
	# Config file installation for server program
	echo "* Installing config files for server at ${SERVER_CONFIG_FILE_PATH}..."
	sudo mkdir -p ${SERVER_CONFIG_FILE_PATH} || { printf "[${COLOR_RED}ERROR${NO_COLOR}] directory creation at ${SERVER_CONFIG_FILE_PATH}\n"; exit -1 ; }
	sudo cp ${SERVER_REPO_CONFIG} ${SERVER_CONFIG} || { printf "[${COLOR_RED}ERROR${NO_COLOR}] Server config installation failed!\n"; exit -1 ; }
	# Config files should have rw-r--r-- permissions, and be owned by root:root
	sudo chown root:root ${SERVER_CONFIG} || { printf "[${COLOR_RED}ERROR${NO_COLOR}] chown failed!\n"; defer_installation ; } 
	sudo chmod 644 ${SERVER_CONFIG} || { printf "[${COLOR_RED}ERROR${NO_COLOR}] chmod failed!\n"; defer_installation ; } 

	# Create the default key file
	sudo cp ${REPO_KEY_FILE} ${ETC_KEY_PATH} || { printf "[${COLOR_RED}ERROR${NO_COLOR}] Server key installation failed!\n"; exit -1 ; }
	sudo chown root:${SYSTEM_USER} ${ETC_KEY_PATH} || { printf "[${COLOR_RED}ERROR${NO_COLOR}] chown failed!\n"; defer_installation ; } 
	sudo chmod 640 ${ETC_KEY_PATH} || { printf "[${COLOR_RED}ERROR${NO_COLOR}] chmod failed!\n"; defer_installation ; } 

	printf "${COLOR_GREEN}SUCCESS${NO_COLOR}: server config files installed properly at ${SERVER_CONFIG_FILE_PATH}\n"

}

create_client_config_files(){
	# Config file installation for client program
	echo "* Installing config files for client at ${CLIENT_CONFIG_FILE_PATH}..."
	mkdir -p ${CLIENT_CONFIG_FILE_PATH} || { printf "[${COLOR_RED}ERROR${NO_COLOR}] directory creation at ${CLIENT_CONFIG_FILE_PATH}\n"; exit -1 ; }
	cp ${CLIENT_REPO_CONFIG} ${CLIENT_CONFIG} || { printf "[${COLOR_RED}ERROR${NO_COLOR}] Client config installation failed!\n"; exit -1 ; }
	# Config files should have rw-r----- permissions, and be owned by user:user
	#sudo chown root:root ${CLIENT_CONFIG} || { printf "[${COLOR_RED}ERROR${NO_COLOR}] chown failed!"; defer_installation ; } 
	sudo chmod 640 ${CLIENT_CONFIG} || { printf "[${COLOR_RED}ERROR${NO_COLOR}] chmod failed!\n"; defer_installation ; } 

	printf "[${COLOR_GREEN}SUCCESS${NO_COLOR}] client config files installed properly at ${CLIENT_CONFIG_FILE_PATH}\n"

}


# check if daemon is already installed in the system,
# otherwise install binary on installation path
# and create file to store chat log
install_daemon(){
	
	# check if daemon is already istalled
	[ -f ${INSTALLATION_FILE} ] && { echo "daemon is already installed in the system."; exit 0; }

	# if it is not installed, then compile and test
	echo "* Compilling and testing..."
	make clean

	make test || { printf "[${COLOR_RED}ERROR${NO_COLOR}] Server compilation/test failed!\n"; exit -1 ; }

	make server
	
	echo "* Installing daemon at ${INSTALLATION_FILE}..."
	sudo mkdir -p ${INSTALLATION_PATH} || { printf "[${COLOR_RED}ERROR${NO_COLOR}] directory creation at ${INSTALLATION_PATH}\n"; exit -1 ; }
	
	# Copy daemon to installation path
	sudo cp ${SERVER_BIN_PATH} ${INSTALLATION_FILE} || { printf "[${COLOR_RED}ERROR${NO_COLOR}] Binary installation failed!\n"; exit -1 ; }

	# Copy other necessary binary files to installation path
	sudo cp ${TERM_BIN_PATH} ${INSTALLATION_TERM} || { printf "[${COLOR_RED}ERROR${NO_COLOR}] Binary installation failed!\n"; exit -1 ; }

	# Change file ownership to root, only root can modify executable
	# root and papayachat con execute file
	# If any of this commands fails, remove binary (security risk!)
	sudo chown root:${SYSTEM_USER} ${INSTALLATION_FILE} || { printf "[${COLOR_RED}ERROR${NO_COLOR}] chown failed!\n"; defer_installation ; } 

	sudo chown root:${SYSTEM_USER} ${INSTALLATION_TERM} || { printf "[${COLOR_RED}ERROR${NO_COLOR}] chown failed!\n"; defer_installation ; } 

	# Change file permission, so that only root and papayachat con execute
	sudo chmod 750 ${INSTALLATION_FILE} || { printf "[${COLOR_RED}ERROR${NO_COLOR}] chmod failed!\n"; defer_installation ; } 

	sudo chmod 750 ${INSTALLATION_TERM} || { printf "[${COLOR_RED}ERROR${NO_COLOR}] chmod failed!\n"; defer_installation ; } 
	
	printf "${COLOR_GREEN}SUCCESS${NO_COLOR}: daemon ${DAEMON_EXECUTABLE_NAME} installed properly!\n"
		
}

create_chat_log(){

	# check if chatlog already exists and is a regular file
	[ -f ${CHATLOG_FILE} ] && return 0
	
	echo "* Creating chat log file at ${CHATLOG_FILE}..."

	sudo mkdir -p ${CHATLOG_PATH} || { printf "[${COLOR_RED}ERROR${NO_COLOR}] directory creation at ${CHATLOG_PATH}"; exit -1 ; }
	
	sudo touch ${CHATLOG_FILE} || { printf "[${COLOR_RED}ERROR${NO_COLOR}] file creation failed!"; exit -1 ; }

	# Change file ownership
	sudo chown root:${SYSTEM_USER} ${CHATLOG_FILE} || { printf "[${COLOR_RED}ERROR${NO_COLOR}] chatlog chown failed!"; defer_installation ; } 
	
	# only read/write permissions for root and group
	sudo chmod 660 ${CHATLOG_FILE} || { printf "[${COLOR_RED}ERROR${NO_COLOR}] chatlog chmod failed!"; defer_installation ; } 

	printf "${COLOR_GREEN}SUCCESS${NO_COLOR}: chatlog created properly!\n"

}

# create configuration files for client
preconfiguration_client(){
	
	create_client_config_files

}

# create configuration files for server
preconfiguration_server(){
	
	check_distribution

	create_system_user

	create_server_config_files

	# create chat log
	create_chat_log

}

install_server(){
	
	# Remove any previous version, before starting installation process
	uninstall --server

	preconfiguration_server
	
	# copy daemon executable to installation path
	install_daemon

	exit 0
	
}

install_client(){

	# Remove any previous version, before starting installation process
	echo "* Removing any previous versions of the client..."
	uninstall --client
	# Create config files for client
	echo "* Copying config files for client..."
	preconfiguration_client
	# Compile client's executable
	echo "* Compilling and installing client's executable..."
	make client || { printf "[ERROR] Compillation of client\n" ; exit -1 ; }
	# Make a bin directory in user's home directory, just in case it does not
	# exist yet. This directory is normally in the path of the shell
	# So that one can then execute the client just by typing 'papayachat'
	mkdir -p ${HOME}/bin 
	cp ./bin/client.bin ${CLIENT_INSTALLATION_FILE} || { printf "[ERROR] Copy client's executable to directory in path\n" ; exit -1 ; }
	printf "[${COLOR_GREEN}SUCCESS${NO_COLOR}] Installation finished!\n"
	exit 0


}

run_server(){

	# check if daemon is already istalled, if so, run papayachat as system user
	[ -f ${INSTALLATION_FILE} ] && sudo -u ${SYSTEM_USER} ${INSTALLATION_FILE} && printf "[${COLOR_GREEN}UP${NO_COLOR}] Executing papayachatd as UID=papayachat\n"

	exit 0

}

print_usage(){
	
	local PROGNAME=$(basename $0)
	echo "${PROGNAME} usage:  ${PROGNAME} [--config] [-c |--client] [-s|--server] [-h|--help] [-u|--uninstall] [-r|--run] [-k|--kill]

	-c --client		Install the client for the chat, if the client is already installed, it un-installs it and installs a new version
	--config		Create config files
	-h --help 		Display usage/man page
	-k --kill 		Kill the daemon
	-r --run 		Run the daemon
	-s --server		Install the daemon/server, if the server is already installed, it un-installs it and installs a new version
	-u --uninstall 	Un-install both client and server from the system
	"

	exit 0

}

[ "$1" = '-c' ] && install_client
[ "$1" = '--client' ] && install_client
[ "$1" = '-s' ] && install_server
[ "$1" = '--server' ] && install_server
[ "$1" = '-h' ] && print_usage
[ "$1" = '--help' ] && print_usage
[ "$1" = '-u' ] && { uninstall --client; uninstall --server ; exit 0; }
[ "$1" = '--uninstall' ] && { uninstall --client ; uninstall --server ; exit 0; }
[ "$1" = '-r' ] && run_server
[ "$1" = '--run' ] && run_server
[ "$1" = '-k' ] && { kill_daemon; exit 0; }
[ "$1" = '--kill' ] && { kill_daemon; exit 0; }
[ "$1" = '--config' ] && { preconfiguration; exit 0; } # only create config files and exit
print_usage
#main_installation
