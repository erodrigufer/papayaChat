## General
* [ ] Re-arrange filesystem of project into folders
* [ ] **Issue with syslog** When running in the cloud service, it gets logged to syslog (problem with config file of syslog instance). It should be all written to a specific file for server daemon
## configureSyslog.c
* [X] Create the configure syslog file or transfer the function to the concurrent server file, because it is needed there.
* [X] ~Handle error of openlog~ openlog does not return a value (void)
* [X] Change API, so that function configureSyslog() accepts parameters with different options
## Makefile
* [X] Makefile should compile both logDaemon and concurrent_server
## basics.h
* [ ] Keep commenting file, remove unnecessary parts
## error_handling.c
* [ ] Keep commenting file, remove unnecessary parts, not all functions have been commented
## inet_sockets.c
### inetConnect()
* [ ] Change errno value modification
## concurrent_server.c
### main()
* [ ] question legitimacy of exit after failed accept() call in server

