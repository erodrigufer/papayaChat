#!/bin/sh
# compile_c.sh

# use 1st parameter of bash script as name for compiled file
# user input can also be none, then use standard file output name
USER_INPUT=$1

# This variable lists all files that will be compiled together, which are
# all C-files in this directory
# The ls programm is written in parenthesis, so that exactly ls is executed
# and not an alias of ls like "ls -1"
FILES_TO_COMPILE=$("ls" *.c)

echo $FILES_TO_COMPILE

[ -z "$USER_INPUT" ] && echo "User input empty!"
[ -n "$USER_INPUT" ] && echo "User input not-empty!"
