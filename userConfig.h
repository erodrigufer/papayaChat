/* userConfig.h

User-defined config file
Use this file to protect sensitive information, not sync
with the open git repository

*/

#ifndef USERCONFIG_H	/* header guard */
#define USERCONFIG_H

/* Define TCP/IP services */
#define SERVICE "51000" /* Port to connect to with server */
//#define HOST "localhost" /* server address (found, e.g. on /etc/hosts file) */
#define HOST "payaserver" /* server address (found, e.g. on /etc/hosts file) */
//#define HOST "kah" /* server address (found, e.g. on /etc/hosts file) */

/* Define path of chat log file */
#define CHAT_LOG_PATH "./chat_log"

#endif
