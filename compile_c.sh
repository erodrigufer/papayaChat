#!/bin/sh
# compile_c.sh
# This shell script compiles all .c files in the working directory together into a file named
# as the first parameter passed to this script
# It compiles activating all warning (-Wall)

# use 1st parameter of bash script as name for compiled file
# user input can also be none, then use standard file output name: a.out
USER_INPUT=$1

compile_with_standard_name() {

	gcc -Wall *.c

}

compile_with_custom_name() {

	gcc -Wall -o "$USER_INPUT" *.c

}

# User input is empty, then compile with standard name
[ -z "$USER_INPUT" ] && compile_with_standard_name

# User input is not empty, then compile with custom (user input) name
[ -n "$USER_INPUT" ] && compile_with_custom_name


exit 0