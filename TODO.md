## General
* [ ] Re-arrange filesystem of project into folders
## configureSyslog.c
* [X] Create the configure syslog file or transfer the function to the concurrent server file, because it is needed there.
* [ ] Handle error of openlog
* [ ] Change API, so that function configureSyslog() accepts parameters with different options
## Makefile
* [X] Makefile should compile both logDaemon and concurrent_server
## basics.h
* [ ] Keep commenting file, remove unnecessary parts
## error_handling.c
* [ ] Keep commenting file, remove unnecessary parts, not all functions have been commented
## ename.c.inc
* [X] Produce file for ubuntu cloud server error names
## inet_sockets.c
### inetConnect()
* [ ] Change errno value modification
## concurrent_server.c
* [X] file uses error-handling functions from the book that have not been implemented!! Compilation will fail! **4.10.2021**
### main()
* [ ] sigemptyset() function should actually be handled check if an error ocurred! with an if-statement. But wait for a deployment in the server to perform this change
* [ ] question legitimacy of exit after failed accept() call in server

