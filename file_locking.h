/* file_locking.h  */

#ifndef FILE_LOCKING_H 		/* header guard */
#define FILE_LOCKING_H

/* open (or if non-existent, create) central chat log file */
int openChatLogFile(void);
int exclusiveWrite(int, char *, size_t);
int sharedRead(int, char*, size_t, off_t);
off_t messagesFromFirstClientConnection(int, char*, size_t, int);
#endif
/* Eduardo Rodriguez 2021 (c) (@erodrigufer). Licensed under GNU AGPLv3 */
