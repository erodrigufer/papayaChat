# papayaChat

[![C Build (CI)](https://github.com/erodrigufer/papayaChat/actions/workflows/c_build.yml/badge.svg?branch=main&event=push)](https://github.com/erodrigufer/papayaChat/actions/workflows/c_build.yml)

Self hosted terminal chat service for the cloud written in C.

## Build and execution
### Back-end
To build and run the back-end executable:
1. `make concurrent_server` 
2. [If necessary change permissions of executable `chmod +x concurrent_server`] Run executable `./concurrent_server`

### Front-end
To build and run the front-end executable:

0. Check that the file *frontEnd.c* connects to the same IP and Port that you defined for your server on the file *concurrent_server.c*
1. `make frontEnd`
2. [If necessary change file permissions of executable `chmod +x frontEnd`] Run executable `./frontEnd`

## Debugging with strace
Useful debugging commands for syscall profilling with **strace**
```
strace -f -T -o <output_file> <process_to_run>

-f : Trace child processes as well
-T : Show relative time of syscalls
-o : Output file

```

## Debugging with tshark
Capture packages with **tshark**
```
sudo tshark -i 1 -P -x -f "tcp port 51000" > ${OUTPUT_FILE}

-i : Capture on interface 1 (runs smoothly in Linode)
-P : (not sure) print summary
-x : Display content of packages in hex
-f : Filter, e.g. only tcp packets on port 51000

```
