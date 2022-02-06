#include "../configParser.h"

/* the first argument passed (argv[1]) will be the file being parsed
the second argument (argv[2]) will be the name of the value being parsed 
if the process fails, the program returns != 0 */
int 
main(int argc, char *argv[])
{
	/* store parsed value here */
	char * value = (char *) malloc(512);
	/* if malloc fails, it returns a NULL pointer */
	if(value == NULL)
		return -1;

	int result = parseConfigFile(argv[1], argv[2], value);
	/* if function does not fail, print value parsed */
	if(result!=-1)
		printf("%s\n",value);

	/* if process fails result != 0 */
	return result;

}
