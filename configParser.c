/* configParser.c
	Defines functions used to parse config files
*/
#include "basics.h"

/* Required for open(2) 
(next three headers) */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* open the config file found at pathname, it returns fd of file if 
file is opened correctly. If not, it returns -1 */
int
openConfigFile(const char * pathname)
{

	/* Define flags for open(2)
	-open for READ only
	-CLOSE file descriptor on EXEC 
	*/
	int flags = O_RDONLY | O_CLOEXEC ;

	/* return fd of openned file, or -1 if error */
	return open(pathname,flags);

}


/* Eduardo Rodriguez 2021 (c) (@erodrigufer). Licensed under GNU AGPLv3 */
