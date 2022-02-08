/* configParser.c
	Defines functions used to parse config files
*/
#include "basics.h"
#include "configParser.h"
#include "CONFIG.h"

/* open the config file found at pathname, it returns FILE pointer,
if file is opened correctly. If not, it returns NULL
If the file does not exist fopen fails */
static FILE *
openConfigFile(const char * pathname)
{

	/* fopen fails if the config file does not exist */
	FILE * fs = fopen(pathname, "r");

	if (fs==NULL)
		return NULL; /* errno is set, so print error with strerror
		and output to syslog */

	return fs;

}

/* parses a single line, looking for a particular configParameter, returns the 
configParameter value if successful */
static int
parseLine(FILE * fs, char * buffer, const char * configParameter, char * returnValue)
{

	/* store content of line into buffer */
	if(fgets(buffer, MAX_LINE_LENGTH, fs)==NULL)
		return -1; /* an error happened, or EOF
		was encountered, return a NULL pointer */

	/* allocate for value to compare with configParameter,
	in theory it could be MAX_LINE_LENGTH long */
	char compareValue[MAX_LINE_LENGTH];
	/* if we get any number other than 2, we did not get the correct
	number of values back from sscanf */
	if(sscanf(buffer,"%s %s", compareValue, returnValue)!=2)
		return 1; /* the line does not contained, two white-space separated
		strings, go to next line */

	if(strncmp(compareValue,configParameter,MAX_LINE_LENGTH)!=0)
		return 1; /* the strings are not the same, go to next line */
	

	return 0; /* we found the right parameter, and we parsed the returnValue */

}

/* main function to parse the value for configParameter  out of config file,
if successful it returns 0, and the value can be found in returnValue,
if it fails, or there is an error, it returns -1 */
int parseConfigFile(const char * pathname, const char * configParameter, char * returnValue)
{
	/* open config file */
	FILE *fs = openConfigFile(pathname);
	if(fs==NULL) /* error opening file */
		return -1;

	/* allocate memory on each for-loop to read message
	from pipe */
	char * buffer = (char *) malloc(MAX_LINE_LENGTH);
	/* if malloc fails, it returns a NULL pointer */
	if(buffer == NULL)
		return -1;

	/* main for-loop to parse each line */
	for(;;){	
		int stop = parseLine(fs, buffer, configParameter, returnValue);
		if(stop == 1)
			continue; /* parse next line */
		if(stop == -1){
			free(buffer);
			fclose(fs);	/* close stream */
			return -1; 	/* error or EOF */
		}

		free(buffer);
		fclose(fs);	/* close stream */
		return 0;	/* parsed correctly, parsed value can be found in returnValue */
	}

}


/* Eduardo Rodriguez 2021 (c) (@erodrigufer). Licensed under GNU AGPLv3 */
