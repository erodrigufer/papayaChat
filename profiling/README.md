# Profiling
## Motivation
A profile of the current papayachat **multi-process** implementation will be measured under different loads and compared with an HTTP server written in Go. The motivation behind this is to see what performance difference there is between a **multi-process** server implementation (papayachat) and a server running with a _non-preemptive_ scheduler, i.e. **coroutines** (Go HTTP server).

### Hypothesis/Open questions
* **How costly is the overhead of kernel-space context switches?** One would assume that the overhead of a multiprocess network application, that has to handle constant context switches through the kernel, would be significantly bigger than in a Go network application, since the Go scheduler tries to handle as many operations as possible in user-space. The difference should be seen in the CPU load of both applications, depending on how much load is applied to the application.
* Parallelization: Can one see a difference in the overall CPU load with an increase of the CPU cores while comparing both approaches? **Does having more cores make one implementation more efficient than the other one?**

## Framework
* Both applications will be tested in the same cloud VMs running _Ubuntu 21.10 (GNU/Linux 5.13.0-30-generic x86_64)_ with **1 vCPU, 2vCPU and 4vCPU**. 
* To measure the difference in CPU load both applications will establish a listening port and subsequently accept a given number of clients that will attempt to communicate with the server concurrently (different numbers of concurrent clients will therefore correspond to different loads).

## Profiling C programs

