#!/bin/bash

# Calculate mean value and standard deviation of 
# every measurement
analyzeData(){
	# The 1st parameter of the function is the file that should be processed.
	local INPUT_FILE=$1

	# The 2nd parameter of the function is the file where the processed data
	# should be stored.
	local OUTPUT_FILE=$2

	awk 'BEGIN {mean=0; count=0} {mean+=$1;count+=1} END {print "Mean: ", mean/count; print NR, " measurements processed."}' ${INPUT_FILE} 

}

CLEANDATA_PATH="./measurements/15_05_2022/cleanData"

cd ${CLEANDATA_PATH}

FILES=$(ls)

for FILE in $FILES; do
	# If FILE is a regular file (to avoid iterating over directories).
	if [ -f ${FILE} ]; then
		OUTPUT_FILE=../../results
		analyzeData ${FILE} ${OUTPUT_FILE} && echo "Processed filed: ${FILE}"
	fi
done

