/* CONFIG.h

File defines IP and port of chat service server 
SERVICE is the PORT number
HOST is the server's IP

It also defines the path where to store the central chat log file
CHAT_LOG_PATH
*/

#ifndef CONFIG_H /* add header guard */
#define CONFIG_H

/* Define TCP/IP services */
#define SERVICE "51000" /* Port to connect to with server */
//#define SERVICE "50000" /* Port to connect to with server */
//#define HOST "localhost" /* server address (found, e.g. on /etc/hosts file) */
//#define HOST "payaserver" /* server address (found, e.g. on /etc/hosts file) */
#define HOST "kah" /* server address (found, e.g. on /etc/hosts file) */

/* Define path of chat log file */
#define CHAT_LOG_PATH "./chat_log"
#endif
