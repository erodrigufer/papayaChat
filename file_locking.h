/* file_locking.h  */

#ifndef FILE_LOCKING_H 		/* header guard */
#define FILE_LOCKING_H

/* open (or if non-existent, create) central chat log file */
int openChatLogFile(void);
int exclusiveWrite(int,ssize_t);
#endif
/* Eduardo Rodriguez 2021 (c) (@erodrigufer). Licensed under GNU AGPLv3 */
