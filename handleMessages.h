/* handleMessages.h

*/

#ifndef HANDLEMESSAGES_H /* header guard */
#define HANDLEMESSAGES_H

/* send message to a pipe */
void sendMessageToPipe(int pipe_fd, char *message);

/* send a message to the server_fd, the message is received through 
a pipe from the parent process */
void handleSendSocket(int server_fd, int pipe_fd);

/* fetch messages from pipe (fetch, but do not print message yet)
the pipe should be configured as O_NONBLOCK
since if the read() on the pipe blocks, all the CLI stalls */
int fetchMessage(int pipe_fd, char *string_buf);

/* read from server socket and pass data through pipe to frontEnd parent process */
void handleReadSocket(int server_fd, int pipe_fd);


#endif

/* Eduardo Rodriguez 2021 (c) (@erodrigufer). Licensed under GNU AGPLv3 */
