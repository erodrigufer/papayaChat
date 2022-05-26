#!/bin/bash

# dataPipeline.sh processes all the CPU load data from the files in a folder. 
# It calculates the mean value and the variance and then plots the data.
#
# Eduardo Rodriguez (@erodrigufer) (c) 2022.
# Licensed under AGPLv3.

###########################################################
#User-defined variables####################################
# Path with all the measurements to analyze.
RESULTS_PATH="./measurements/15_05_2022"

###########################################################
#Global variables##########################################
# Store current path, where this script is being executed.
CURRENT_PATH=$(pwd)
PLOTTER_EXECUTABLE="plotCurves.bin"
###########################################################


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

# Compile the go program in charge of plotting all the data and calculating the
# mean values, variance and so on.
buildPlotter(){
	cd ./goPlotting/plotCurves
	go build -o ../../${PLOTTER_EXECUTABLE} plotCurves.go && echo "Successfully built executable for plotter." || { echo "Failed at building plotter."; return -1; }
	cd ../../
	return
}


main(){

	cd ${RESULTS_PATH}
	# Remove any previous iteration of results and clean data in the same path.
	rm -rf "cleanData"
	rm -rf "results"
	mkdir -p "cleanData" 	# Store cleaned data here.
	mkdir -p "results" 		# Store plots and other results here.	

	# Store results of 'ls' command.
	FILES=$(ls)

	for FILE in $FILES; do
		# If FILE is a regular file (to avoid iterating over directories).
		if [ -f ${FILE} ]; then
			# Where to store clean data.
			OUTPUT_FILE=./cleanData/${FILE}.clean
			processFile ${FILE} ${OUTPUT_FILE} && { echo "Cleaned data from file: ${FILE}"; }
			../../plotCurves.bin -input ${OUTPUT_FILE} -plot ./results/${FILE}.pdf -output ./results/results.txt
		fi
	done
}

buildPlotter || exit -1
main
