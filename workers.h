#ifndef WORKERS_H
#define WORKERS_H

#include <pthread.h>
#include <stdio.h>
#include <sys/poll.h>

#include "list.h"
///THREAD JOBS
#define SUP 100			//supervisor
#define IO 200			// Local data IO
#define LBR 300			// Labourers ,encrypt outgoing or decrypt incoming and pass it to IO threads

static int show_stats=0,interval=2;
struct thread_management {
  pthread_t *T;
  void *arg;
  int job;
  int id;
  int running;
  struct list_head TL;

};
struct thread_management MTL;	//Master Thread List

struct thread_stats {
  int gtc;
  int supc;
  int ioc;
  int lbrc;
};
struct thread_stats GTS;	//Global thread thread_stats

/***************************************************************
 * threads
 * these two handle traffic to/from local enumerate_interfaces
 *  such as tun interface,unix socket or named pipe
 *  we need to start just one of these per interface
 *  they are responsible for chopping up and pushing traffic into their queue
 ********************************************************************/
void *tx_io (void *a);
void *rx_io (void *a);
void *map_in (void *a);

void *tx_lb(void *a);
void *rx_lb(void *a);
void *map_lb(void *a);
/***************************************************************************
 * at least 1 per interface
 * per interface configurable thread count.
 * this means you can have 4 of these on one interface and 2 on others.
 * 
 * They are  responsible for getting data out of the ring buffer
 * and encrypting/decrypting it as well as sending it off to the remote/local 
 * network interface.
 * **************************************************************************/

void *tx (void *a);
void *rx (void *a);
void *map (void *a);



/**** background/cleanup tasks,get counter info,mesure performance,etc...
 * runs 2 seconds collecting information and possibly acting on it.
 * ************************************************************************/
void *supervisor (void *a);



#endif
