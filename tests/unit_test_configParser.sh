#!/bin/sh

ALL_TESTS_PASSED=true

PARSER_EXECUTABLE=parser.bin

FILE2PARSE=ut_configParser_golden_reference

compile_parser(){
	gcc -c ../configParser.c
	gcc -o ${PARSER_EXECUTABLE} ./test_configParser.c ./configParser.o
}

clean(){
	rm ./${PARSER_EXECUTABLE}
	rm ./*.o
}

parse(){

	./${PARSER_EXECUTABLE} ${FILE2PARSE} $1

}

parse_test_unit(){
	
	# word to be parsed
	local WORD=$1
	# expected outcome
	local EXPECTATION=$2

	local PARSED_RESULT=$(parse ${WORD})

	# test if parsed result equals expectation	
	[ "${PARSED_RESULT}" = "${EXPECTATION}" ] && printf "[passed] ${WORD} parsed correctly...\n" || { printf "[FAILED] ${WORD} should result in ${PARSED_RESULT}, but result was ${EXPECTATION}...\n" ; ALL_TESTS_PASSED=false ; }

}

compile_parser

parse_test_unit PORT 51000
parse_test_unit HOST papayachat
parse_test_unit NON-EXISTANT

# remove executable compiled before
clean

[ "${ALL_TESTS_PASSED}" = "true" ] && { printf "[SUCCESS] All tests passed! \n" ; exit 0 ; } || { printf "[FAILURE] Some test(s) failed! \n" ; exit -1 ; }
