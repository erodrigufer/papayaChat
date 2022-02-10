# papayaChat

[![C Build (CI)](https://github.com/erodrigufer/papayaChat/actions/workflows/build.yml/badge.svg)](https://github.com/erodrigufer/papayaChat/actions/workflows/build.yml)
[![Networking (CI)](https://github.com/erodrigufer/papayaChat/actions/workflows/networking.yml/badge.svg)](https://github.com/erodrigufer/papayaChat/actions/workflows/networking.yml)
[![Unit-testing (CI)](https://github.com/erodrigufer/papayaChat/actions/workflows/unit-tests.yml/badge.svg)](https://github.com/erodrigufer/papayaChat/actions/workflows/unit-tests.yml)

Self-hosted CLI chat service for the cloud written in C, the client terminal interface is portable accross Unix systems (Mac OS, Linux).

## Installation guide

### Back-end (server/daemon)
Step by step guide to install the server/daemon that will handle the chat service:
1. `make install-server` 
	- You need sudo rights to install the daemon in your system. 
	- This command will create a system user named `papayachat`, which will run the papayachat daemon _papayachatd_ in the background.
	- The daemon's executable will be compiled and tested before being placed at `/usr/local/bin/papayachat/`. 
	- The chatlog's file can be found at `/var/lib/papayachat/`. 
	- If there is an error during the compilation or the build does not pass the _networking_ test, the daemon will not be installed on the system.
2. Change the server's configuration file (`/etc/papayachat/server.config`) to suit your needs
	- You need sudo rights to modify the config files and server's key.
	- Specify the `PORT` you would like to use for your chat service. _papayachatd_ daemon will be **listening** on this port for any clients wishing to connect to the service.
3. Generate a new key and place it in the configuration folder for the server
	- The file should be placed on `/etc/papayachat/` with the name `key`
	- After the installation there will be a default key inside the config folder.
	- The key should have the following permission rw-r----- root:papayachat, so that the daemon can read its contents
	- The key should be exactly 128 characters long. It is recommended to generate it using a SHA-512 hash.
4. `make run` to run the chat service as a daemon
	- You can verify that the daemon is running with the following command: `ps -ef | grep papayachat` which should display the running process of the daemon, or with `ss -at` which should display that a service is **listening** at the `PORT` you choosed for the service.  

### Further remarks
* `make kill` will stop the papayachat daemon, if it is currently running.
* `make uninstall` will shutdown any running papayachat daemon and **un-install** it and any files related to both the client and the server
* To **upgrade** to a newer version:
	1. Get the release you want to upgrade
	2. `make install-server` will un-install you current server executables, chat logs and config files, and install the version from the local repo.

### Client
Step by step guide to install the client:
1. `make install-client`
	- This command installs the client executable at `~/bin/papayachat` so that afterwards by simply typing `papayachat` in your terminal, the chat client should be executed.
	- A configuration file for the client is also installed at `~/.papayachat/client.config`.
2. Change the client's configuration file (`~/.papayachat/client.config`) to suit your needs
	- You should change the default **username** and **key**. And specify the **port** and **IP address** of the server running the service you want to connect to.
	- You should get the key from the person administrating the papayachat server before trying to connect to the service. Otherwise, the authentication will fail (check [the section about installing the server](back-end-(server/daemon)) to learn more about how to create a key).

### Further remarks
* If you have done everything right so far, reboot your terminal to definitely be able to see the papayachat executable in your _path_. After that, you should be able to start the client simply by running the command `papayachat` in your terminal.
* `make uninstall` will shutdown any running papayachat daemon and **un-install** it and any files related to both the client and the server
* To **upgrade** to a newer version:
	1. Get the release you want to upgrade
	2. `make install-client` will un-install you current client executable and config file, and install the version from the local repo.


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
