#!/bin/sh
# compile_c.sh

# use 1st parameter of bash script as name for compiled file
# user input can also be none, then use standard file output name
USER_INPUT=$1

# This variable lists all files that will be compiled together, which are
# all C-files in this directory
# The ls programm is written in parenthesis, so that exactly ls is executed
# and not an alias of ls like "ls -1"
FILES_TO_COMPILE=$(ls -C prueba.c)

# TO DO: change FILES_TO_COMPILE with awk or something, so that each file name gets its PWD

echo $PWD

cd $PWD

compile_with_standard_name() {

	gcc -Wall "${PWD}/${FILES_TO_COMPILE}"

}

compile_with_custom_name() {

	gcc -Wall -o "$USER_INPUT" "$FILES_TO_COMPILE"

}

# User input is empty, then compile with standard name
[ -z "$USER_INPUT" ] && compile_with_standard_name

# User input is not empty, then compile with custom (user input) name
[ -n "$USER_INPUT" ] && compile_with_custom_name


exit 0
