# papayaChat

[![C Build (CI)](https://github.com/erodrigufer/papayaChat/actions/workflows/build.yml/badge.svg)](https://github.com/erodrigufer/papayaChat/actions/workflows/build.yml)
[![Networking (CI)](https://github.com/erodrigufer/papayaChat/actions/workflows/networking.yml/badge.svg)](https://github.com/erodrigufer/papayaChat/actions/workflows/networking.yml)
[![Unit-testing (CI)](https://github.com/erodrigufer/papayaChat/actions/workflows/unit-tests.yml/badge.svg)](https://github.com/erodrigufer/papayaChat/actions/workflows/unit-tests.yml)

Self-hosted CLI chat service for the cloud written in C, the client terminal interface is portable accross Unix systems (Mac OS, Linux).

## Installation guide

### Back-end
Step by step guide:
1. `make install-server` 
	You need sudo rights to install the daemon in your system. This command will copy the daemon executable
To build and install the back-end executable:
* `make install-server` (you need sudo rights to install the chat daemon in your system).
This will create a system user named `papayachat` which will run the papayachat daemon _papayachatd_ in the background. The daemon's executable will be compiled and tested before being placed at `/usr/local/bin/papayachat/`. Meanwhile, the chatlog's file can be found at `/var/lib/papayachat/`. If there is an error during the compilation or the build does not pass the _networking_ test, the daemon will not be installed on the system.

After installing the back-end, to start running the daemon:
* `make run`
You can further check that the daemon is running with the command `ss -at`, it should show you a service _listening_ at the port you specified for the **papayachatd**.

To upgrade the current version of papayachat in the system:
* `make upgrade`
This will take down any running instance of the daemon, remove its executable and the chatlog file from the system, and install again the backend found in the local repository version (always `git pull` before doing this to have the most recent version of the repo locally).

To shutdown any running daemon and uninstall it from the system:
* `make uninstall` 

To shutdown any running daemon:
* `make kill`

### Front-end
**This section is incomplete!**

### Stopping the back end daemon
In order to stop a, possibly, long-running instance of the back-end daemon:

0. You can either simply run `make kill`, or:
1. Find out the PID of the daemon with `ps -ef | grep papayachat`. If there is not an established connection with a client(s), there should be only a single running daemon process (every new client connection creates 2 processes).
2. Kill all running processes owned by **papayachat** (you need sudo rights for this, since the daemon and its child processes are running as system user _papayachat_).

## Debugging
Some useful tips for debugging low level system calls and/or networking issues.

### Debugging with strace
Useful debugging commands for syscall profilling with **strace**
```
strace -f -T -o <output_file> <process_to_profile>

-f : Trace child processes as well
-T : Show relative time of syscalls
-o : Output file

```
`-f` option is necessary in all executables that use `fork()` to create child processes.

### Debugging with tshark
Capture packages with **tshark**. See traffic flow between server and client.
```
sudo tshark -i 1 -P -x -f "tcp port ${PORT_NUMBER}" > ${OUTPUT_FILE}

-i : Capture on interface 1 (runs smoothly in Linode)
-P : (not sure) print summary
-x : Display content of packages in hex
-f : Filter, e.g. only tcp packets on port 51000

```
