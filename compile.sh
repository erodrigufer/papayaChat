#!/bin/sh

# This variable lists all files that will be compiled together, which are
# all C-files in this directory
# The ls programm is written in parenthesis, so that exactly ls is executed
# and not an alias of ls like "ls -1"
FILES_TO_COMPILE=$("ls" *.c)

echo $FILES_TO_COMPILE

