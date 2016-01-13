#ifndef RING_H
#define RING_H
#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "util.h"

 static const uint32_t HEADER=0xf01abcde;
 static const uint32_t FOOTER=0xcafeface;
 typedef struct ring_block {
  uint32_t header;
  uint8_t *footer;
   int len;
   int index;
   volatile int lock;
  uint8_t *ethernet_frame;
  struct iphdr *ipheader;
   uint8_t *data;
} RNG;

typedef struct queue {
 //pthread_mutex_t mutex;
//pthread_spinlock_t spinlock;
    volatile unsigned int id;
    volatile unsigned int bsz;
    volatile unsigned int rsz;
    volatile unsigned int in;
    volatile unsigned int out;
    volatile unsigned int sz;
    volatile unsigned int lock;
    volatile unsigned int spins;
    struct timespec ts;
  RNG *R;

} Q;

inline RNG *push (Q * q);
inline RNG *pop(Q *q);
void init_ring_buffer (Q* q,int size, int chunks, struct headers *hdr);
#endif
