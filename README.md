# papayaChat

[![C Build (CI)](https://github.com/erodrigufer/papayaChat/actions/workflows/build.yml/badge.svg)](https://github.com/erodrigufer/papayaChat/actions/workflows/build.yml)
[![Networking (CI)](https://github.com/erodrigufer/papayaChat/actions/workflows/networking.yml/badge.svg)](https://github.com/erodrigufer/papayaChat/actions/workflows/networking.yml)

Self-hosted CLI chat service for the cloud written in C.

Host your own E2E encrypted chat service, and chat directly from the terminal.

A goal of the proyect is to reliably run the chat client in any Unix system. Nonetheless, as of the current developments, it only runs flawlessly with Linux (Ubuntu-based) distributions. The code is partly portable to Mac OS X.

## Build and execution
**Since the last development sessions, the following build and execution instructions might be outdated. I will update them as soon as I have time. Nonetheless, check the file _run_server.sh_ to see which commands to run for the automated installation, de-installation and execution of the programm.**
### Back-end
To build and run the back-end executable:
1. `make concurrent_server` 
2. [If necessary change permissions of executable `chmod +x concurrent_server`] Run executable `./concurrent_server`

### Front-end
To build and run the front-end executable:

0. Check that the file *frontEnd.c* connects to the same IP and Port that you defined for your server on the file *concurrent_server.c*
1. `make frontEnd`
2. [If necessary change file permissions of executable `chmod +x frontEnd`] Run executable `./frontEnd`

## Stopping the back end daemon
In order to stop a, possibly, long-running instance of the back-end daemon:

1. Find out the PID of the daemon with `ps -ed | grep papayachat`. If there is not an established connection with a client(s), there should be only a single running daemon process (every new client connection creates 2 processes).
2. Kill all running processes owned by **papayachat**

## Debugging with strace
Useful debugging commands for syscall profilling with **strace**
```
strace -f -T -o <output_file> <process_to_profile>

-f : Trace child processes as well
-T : Show relative time of syscalls
-o : Output file

```
`-f` option is necessary in all executables that use `fork()` to create child processes.

## Debugging with tshark
Capture packages with **tshark**. See traffic flow between server and client.
```
sudo tshark -i 1 -P -x -f "tcp port ${PORT_NUMBER}" > ${OUTPUT_FILE}

-i : Capture on interface 1 (runs smoothly in Linode)
-P : (not sure) print summary
-x : Display content of packages in hex
-f : Filter, e.g. only tcp packets on port 51000

```
