Lanes 

Lanes is a virtual tunnel application designed with  scalability,adaptability and flexibility in mind.

Like the name suggests, the tunnels are lanes,meaning uni-directional.

Existing tunnel techniques are required to be bi-directional for traffic to flow. 

With the belief that a uni-directional tunnel is better suited to engineer virtual network 
topologies that are extremely flexible,high performance and resillient, Lanes aims to 
provide an interface to transport and (optionally) encrypt traffic as the user sees it fit.

Simple configuration,  a detailed debugging and management interface and support for 
as many data sources and destinatons as possible are the current user interface goals

We believe a tool is something that should work as simply and in the most efficient way possible.
You should tell a tool what to do, it shouldn't tell you or restrict you on what you can and cannot do.

The primary goal of Lanes is to give it's users more choices and options, not to beat or "one-up" other projects.

--------------------------------------------------------------------------------------

Unless this readme is changed to reflect otherwise, DO NOT use Lanes for production traffic.

- CRITICAL things that do not work and need to work before any version release can happen:

 *  Correct and persistent traffic forwarding ( 72 hours continuous iperf test needs to pass)
 *  Encryption - no encryptoin code has been implemented. the networking needs to work well first
              We need to audit and clean up what already exists and make it work well. 
             We cannot afford to integrate security and encryption on a weak/untested foundation.
 *  UNIX Sockets, named pipes as data sources. 
 *  Application termination/management interface (current plan is via unix sockets and telnet interface in addition to cli)
 *  Propper logging to syslog
 *  SECCOMP 
 *  Proper signal handling 
 *  Line by line review for known code vulns
 *  Thorough gprof and Valgrind tests for code optimization
 *  Packet_MMAP,AF_PACKET fanout, Netmap

- IPV6,locale and support for assymetric cryptography is planned for a future release

-----------------------------------------------------------------------------------------

Make:
Just run "make"

Run:

./lanes ./sample.yaml
The above rules an instance of lanes
./lanes ./sample-remote.yaml
This one rules lanes but uses the "sample-remote.yaml" ,this instance
should be run on a separate machine that can communicate with the first one.

-------------------------------------------------------------------------------------------

ALL help is accepted, Please accompany any technical critique with a pull request whenever possible.

Have fun!
