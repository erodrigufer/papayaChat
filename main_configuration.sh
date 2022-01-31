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

# Some commands are debian-based (even probably BSD compliant)
# so if the distro is not debian-based exit
check_distribution(){
	# print Linux_Standard_Base release ID
	# use grep in egrep mode (-e), being 
	# case-insensitive (-i) and check for either 
	# ubuntu or debian
	echo "* Checking system distribution..."
	lsb_release -i | grep -i -E "(ubuntu|debian|kali)" || { printf "[${COLOR_RED}ERROR${NO_COLOR}] System must be debian-based to run daemon."; exit -1 ; }

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

	return -1

}

uninstall(){
	kill_daemon
	echo "* Uninstalling ${DAEMON_EXECUTABLE_NAME}..."
	sudo rm -rf ${INSTALLATION_PATH} || { printf "[${COLOR_RED}ERROR${NO_COLOR}] daemon executable could not be removed!"; exit -1 ; }
	echo "* Removing chat log file..."
	sudo rm -rf ${CHATLOG_PATH} || { printf "[${COLOR_RED}ERROR${NO_COLOR}] chatlog file could not be removed!"; exit -1 ; }
	echo "All files removed. Done!"
	return 0

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

	echo "* Compilling and testing..."
	# if it is not installed, then compile and test
	make clean	
	make test || { printf "[${COLOR_RED}ERROR${NO_COLOR}] Server compilation/test failed!"; exit -1 ; }

	make server
	
	echo "* Installing daemon at ${INSTALLATION_FILE}..."
	sudo mkdir -p ${INSTALLATION_PATH} || { printf "[${COLOR_RED}ERROR${NO_COLOR}] directory creation at ${INSTALLATION_PATH}"; exit -1 ; }
	
	# Copy daemon to installation path
	sudo cp ${SERVER_BIN_PATH} ${INSTALLATION_FILE} || { printf "[${COLOR_RED}ERROR${NO_COLOR}] Binary installation failed!"; exit -1 ; }

	# Copy other necessary binary files to installation path
	sudo cp ${TERM_BIN_PATH} ${INSTALLATION_TERM} || { printf "[${COLOR_RED}ERROR${NO_COLOR}] Binary installation failed!"; exit -1 ; }

	# Change file ownership to root, only root can modify executable
	# root and papayachat con execute file
	# If any of this commands fails, remove binary (security risk!)
	sudo chown root:${SYSTEM_USER} ${INSTALLATION_FILE} || { printf "[${COLOR_RED}ERROR${NO_COLOR}] chown failed!"; defer_installation ; } 

	sudo chown root:${SYSTEM_USER} ${INSTALLATION_TERM} || { printf "[${COLOR_RED}ERROR${NO_COLOR}] chown failed!"; defer_installation ; } 

	# Change file permission, so that only root and papayachat con execute
	sudo chmod 750 ${INSTALLATION_FILE} || { printf "[${COLOR_RED}ERROR${NO_COLOR}] chmod failed!"; defer_installation ; } 

	sudo chmod 750 ${INSTALLATION_TERM} || { printf "[${COLOR_RED}ERROR${NO_COLOR}] chmod failed!"; defer_installation ; } 
	
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

main_installation(){
	check_distribution

	create_system_user

	# copy daemon executable to installation path
	install_daemon

	# create chat log
	create_chat_log

	return 0
	
}

# Remove previous daemon version from system and upgrade to new one
upgrade(){

	uninstall && main_installation && return 0

}

run_server(){

	# check if daemon is already istalled, if so, run papayachat as system user
	[ -f ${INSTALLATION_FILE} ] && sudo -u ${SYSTEM_USER} ${INSTALLATION_FILE} && printf "[${COLOR_GREEN}UP${NO_COLOR}] Executing papayachatd as UID=papayachat\n"

	exit 0

}

print_usage(){
	
	local PROGNAME=$(basename $0)
	echo "${PROGNAME} usage:  ${PROGNAME} [-h|--help] [-u|--uninstall] [g|--upgrade] [-r|--run] [-k|--kill] [gr]
	
	-h Display usage/man page
	-g Upgrade daemon by uninstalling current daemon and installing current version of daemon inside the repo
	-r Run the daemon
	-k Kill the daemon
	-u Uninstall the daemon binary and chatlog file from the system
	"

	exit 0

}

[ "$1" = '-h' ] && print_usage
[ "$1" = '--help' ] && print_usage
# -u flag, run uninstall
[ "$1" = '-u' ] && { uninstall; exit 0; }
[ "$1" = '--uninstall' ] && { uninstall; exit 0; }
[ "$1" = '-r' ] && run_server
[ "$1" = '--run' ] && run_server
[ "$1" = '-g' ] && { upgrade; exit 0; }
[ "$1" = '--upgrade' ] && { upgrade; exit 0; }
# First upgrade, then run newly upgraded server
[ "$1" = '-gr' ] && upgrade && run_server
[ "$1" = '-rg' ] && upgrade && run_server
[ "$1" = '-k' ] && { kill_daemon; exit 0; }
[ "$1" = '--kill' ] && { kill_daemon; exit 0; }
main_installation
