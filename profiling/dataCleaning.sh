#!/bin/bash


# processFile, extracts the CPU load data out of the raw 'top' output file.
processFile(){
	# The 1st parameter of the function is the file that should be processed.
	local INPUT_FILE=$1

	# The 2nd parameter of the function is the file where the processed data
	# should be stored.
	local OUTPUT_FILE=$2

	# sed replaces 'last...' with '---', then print the lines with '---', and
	# finally also prints the lines with the name of the server being executed
	# 'server.bin'. '-n' flag means do not output all the input file.
	sed -n -e 's/last.*$/---/' -e '/---/p' -e '/server.bin/p' ${INPUT_FILE} | awk 'BEGIN {sum=0} $1 == "---" {if(sum != 0) {print sum}; sum=0} $11 == "server.bin" {sum+=$10}' > ${OUTPUT_FILE}
		# awk initializes the variable sum with the value of 0. Then, if the
		# element in the 1st column equals '---' and sum is not 0, it prints 
		# sum and resets sum to 0. If the element in the 11th column equals 
		# 'server.bin', then it increases sum by the value in the 10th column.
		# This is necessary to add up all the CPU load values of a single time
		# measurement (they are always nested between '---' lines).
}

# Store current path, where this script is being executed.
CURRENT_PATH=$(pwd)

RESULTS_PATH="./measurements/15_05_2022"

cd ${RESULTS_PATH}
mkdir -p "cleanData"

# Store results of 'ls' command.
FILES=$(ls)

for FILE in $FILES; do
	# If FILE is a regular file (to avoid iterating over directories).
	if [ -f ${FILE} ]; then
		OUTPUT_FILE=./cleanData/${FILE}.clean
		processFile ${FILE} ${OUTPUT_FILE} && echo "Processed filed: ${FILE}"
	fi
done

# cd ${CURRENT_PATH}
