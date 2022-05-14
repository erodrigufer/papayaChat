# Profiling

## Table of contents
<!-- vim-markdown-toc GFM -->

* [Motivation](#motivation)
	- [Hypothesis/Open questions](#hypothesisopen-questions)
* [Framework](#framework)
* [Profiling with top](#profiling-with-top)
	- [First test](#first-test)
	- [References](#references)
* [tcpkali as a load generator](#tcpkali-as-a-load-generator)

<!-- vim-markdown-toc -->

## Motivation
A TCP server written in C will be measured under different loads (different number of concurrent connections) and compared with a TCP server written in Go. The motivation behind this, is to see what performance difference (i.e. CPU load) there is between a **multi-process** server implementation (papayachat) and a server running with a _non-preemptive_ scheduler, i.e. **coroutines** (Go TCP server).

### Hypothesis/Open questions
* **How costly is the overhead of kernel-space context switches?** One would assume that the overhead of a **multiprocess** network application, that has to handle constant context switches through the kernel for each concurrent client connection, would be significantly bigger than in a Go network application. Since the Go scheduler tries to handle as many operations as possible in **user-space**, where context-switches tend to be _cheaper_. The difference should be seen in the CPU load of both applications, depending on how many concurrent active connections a server has to handle. **Is there a very palpable difference between the performance of a multi-process server and one with a non-preemptive scheduler when the number of concurrent active connections increases?**
* **Parallelization**: Can one see a difference in the overall CPU load with an increase of the available CPU cores for a server? **Does having more cores make one implementation more efficient than the other one?**

## Framework
* Both server applications will be tested in the same cloud VMs (Vultr) running _FreeBSD 13 x64_ with **1vCPU, 2vCPU and 4vCPU**.
* To measure the difference in CPU load both applications will establish a listening port and subsequently accept a given number of clients that will attempt to communicate with the server concurrently (different numbers of concurrent clients will therefore correspond to different loads).
* The overall CPU load of the system will be measured over a given amount of time after all clients have established a connection with the server.
* The load (packages sent to the server) originates in another cloud VM running `FreeBSD 13 x64` in the same region as the server VM to avoid high latency. The load is generated with `tcpkali` (see [tcpkali's GitHub repo](https://github.com/satori-com/tcpkali)). `tcpkali` is a high performance TCP and WebSocket load generator.

## Profiling with top
As described in Reference #2: 

```bash
top -b -n 10 -d 1 -u papayachat >> <TEXT_OUTPUT_FILE>
	-b : Batch mode (output goes to stdout)
	-d : Delay between captures (in seconds)
	-n : Total number of captures
	-u : Show only processes being run by a particular user 
```

Further exploration of the functionality of top should be made regarding the `-S` flag, to should the cumulative time of the processes (see man page).

Taken from the manpage:

```
38. TIME  --  CPU Time
	Total  CPU time the task has used since it started.  When Cumulative mode 
	is On, each process is listed with the cpu time that it and its dead 
	children have used.  You toggle Cumulative mode with `S', which is both a 
	command-line option and an interactive command.  See the `S' interactive 
	command for  additional	information regarding this mode.

39. TIME+  --  CPU Time, hundredths
	The same as TIME, but reflecting more granularity through hundredths of a 
	second.
```

Meaning of abbreviations:

```
%CPU  --  CPU Usage
	The task's share of the elapsed CPU time since the last screen update, 
	expressed as a percentage of total CPU time.

%Cpu(s):  0.0 us,  0.0 sy,  0.0 ni,  100.0 id,  0.0 wa,  0.0 hi,  0.0 si,  0.0 st
```

```
us, user    : time running un-niced user processes
sy, system  : time running kernel processes
ni, nice    : time running niced user processes
id, idle    : time spent in the kernel idle handler
wa, IO-wait : time waiting for I/O completion
hi : time spent servicing hardware interrupts
si : time spent servicing software interrupts
st : time stolen from this vm by the hypervisor
```

### First test
0. If running in Ubuntu, stop `ufw` before starting the test:
```bash
service ufw stop
```
1. On server (profiler), run `nc` listening on a port:
```bash
nc -l -k <PORT>
``` 
And subsequently (in another window, or run `nc` in the background in the same 
window), run `top` to measure the CPU load:

```bash
Linux: 
top -b -n 20 -d 1 >> <FILE_OUTPUT> 

FreeBSD:  
top -s 0.5 -b -d <#_of_FRAMES> >> <FILE_OUTPUT>

-s: Time in seconds between screen captures (can also be fractional)
```

2. On client (load generator), run `tcpkali` to send a message to the server:
```bash
tcpkali --message "<MESSAGE_STRING>" <IP_PROFILER>:<PORT>
```

3. Filter the data generated by `top` to only see the data generated by `nc`:
```bash
cat <FILE_OUTPUT> | grep " nc$" | less
```

### References
1. [getrusage(2)](https://man7.org/linux/man-pages/man2/getrusage.2.html): Use the syscall getrusage(2) in a C program to measure the amount of time spent executing in both user and kernel mode for either the program itself or its children.
2. [Efficient way to calculate CPU usage of multiple processes in Linux (stackoverflow)](https://stackoverflow.com/questions/34103971/efficient-way-to-calculate-cpu-usage-of-multiple-processes-in-linux)

## tcpkali as a load generator
```bash
tcpkali --write-combine=off --workers <NUMBER OF CORES> \ 
--connections <NUMBER OF CONCURRENT TCP CONNECTIONS> --duration 30s \
--channel-bandwidth-upstream 500Kbps --first-message "conn" \
-em 'message\n' <IP_SERVER>:<PORT>
```
