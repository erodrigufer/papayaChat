/* configParser.h  */

#include "basics.h"	/* otherwise the type FILE pointer is undefined */

#ifndef CONFIGPARSER_H	/* Header guard */
#define CONFIGPARSER_H

static FILE openConfigFile(const char *);

static int parseLine(FILE *, char *, const char *, char *);

int parseConfigFile(const char *, const char *, char *);

#endif
