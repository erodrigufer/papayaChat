/* clientRequest.h

*/

/* in order to define pid_t */
#include "basics.h"

#ifndef CLIENTREQUEST_H	/* header guard */
#define CLIENTREQUEST_H

void handleRequest(int,int);

static void introMessage(int);

static void sendNewMessages(int, int);

static void receiveMessages(int, int, pid_t);

static void killChild(pid_t);

#endif
