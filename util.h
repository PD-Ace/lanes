#ifndef UTIL_H
#define UTIL_H


#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include<stdarg.h>
#include <stdlib.h>
#include <netinet/ether.h>
#include <netinet/ip.h>

#include "workers.h"

#include "main.h"

#include "stdio.h"

#define IFINDEX 0
#define IFMTU 1
#define IFMAC 2
#define IFADDR 3

#define TUN 90
#define TAP 95
#define UNIXSOCKET 100
#define PIPE 110
#define IFMAP 120
//ethernet and arp ...
#define ETH_HDRLEN 14
#define IP4_HDRLEN 20
#define ARP_HDRLEN 28
#define ARP_TIMEOUT 5
#define ETHIP4 IP4_HDRLEN + ETH_HDRLEN


extern char *optarg;
extern int optind, optopt;


struct peer_context MPL;	//Master peer list

void add_to_peer_list (struct peer_context *p);
void add_to_thread_list (struct thread_management *t);
void init_peer_list ();
void init_thread_list ();
void peer_list_dump ();
void die (int really, char *why, ...);
void logg (char *s, ...);
void debug (int lvl, char *s,...);
void drop_privs();
void signal_handler(int sig);

inline int atomic_islocked(volatile int L);
inline int atomic_lock(volatile int *L);
inline int atomic_unlock(volatile int *L);
inline int atomic_cond_lock (volatile int *L);
inline void  adaptive_spin (struct timespec *ts,volatile int watch,volatile int *spins);

struct thread_management *new_thread (int job, void *(*start_routine) (void *), void *arg);
unsigned int g_rand ();
unsigned short csum (unsigned short *buf, int nwords);

struct headers {
  struct ethhdr eh;
  struct iphdr iph;

};
#endif
