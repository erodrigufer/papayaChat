#include "../configParser.h"

/* the first argument passed will be the file parsed
the second argument will be the parameter being parsed */
int 
main(int argc, char *argv[])
{
	char * value = (char *) malloc(512);
	/* if malloc fails, it returns a NULL pointer */
	if(value == NULL)
		return -1;

	int result = parseConfigFile(argv[1], argv[2], value);
	if(result!=-1)
		printf("%s\n",value);

	return result;

}
