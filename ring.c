#include "ring.h"
#include <string.h>

/*
 *   q - handle to the circular ring buffer 'queue'
 *   size - the ring buffer's size in bytes
 *   chunks - block size,for network traffic this would be 'mtu'
 *            the size of the data units in the ring buffer.
 * *******************************************************************/

void
init_ring_buffer (Q * q, int size, int chunks, struct headers *hdr) {
  int i = 0, k = 0, hdrlen = IP4_HDRLEN + ETH_HDRLEN;
  uint8_t *datapool;

  memset (q, 0, sizeof (Q));
  uint8_t *hdr_eth = (uint8_t *) & hdr->eh;
  uint8_t *hdr_ip = (uint8_t *) & hdr->iph;

  q->bsz = chunks+sizeof(uint32_t);
  q->rsz = (size / q->bsz);

  datapool = calloc (q->rsz, q->bsz);
  q->R = calloc (q->rsz, sizeof (RNG));
  
  q->mutex = (pthread_mutex_t *) malloc (sizeof (pthread_mutex_t));
  pthread_mutex_init (q->mutex, NULL);
  
  for (i; i < q->rsz; i++) {
  
    //q->R[i].rmut = (pthread_mutex_t *) malloc (sizeof (pthread_mutex_t));
    if (pthread_mutex_init (&q->R[i].rmut, NULL)) {
      die (1,"MUTEX INIT error");
    }
    q->R[i].header=HEADER;
    q->R[i].ethernet_frame = datapool + (i * q->bsz);
    q->R[i].ipheader = (struct iphdr *) &q->R[i].ethernet_frame[ETH_HDRLEN];
    q->R[i].data = (uint8_t *) & q->R[i].ethernet_frame[hdrlen];
    q->R[i].index = i;
    q->R[i].footer= datapool +(((i+1) * (q->bsz))-sizeof(uint32_t));
    memcpy(q->R[i].footer,&FOOTER,sizeof(uint32_t));
    for (k = 0; k < ETH_HDRLEN; k++) {
      q->R[i].ethernet_frame[k] = hdr_eth[k];
    }

    memcpy (q->R[i].ipheader, &hdr->iph, sizeof (struct iphdr));

  }
}
inline int valid_RNG(RNG *r){
  if(r->header == HEADER && memcmp(r->footer,&FOOTER,sizeof(uint32_t))==0)return 1;
  
  return 0;
}
/*
 
 * q - handle to the ring buffer 
 * 
 * returns NULL if the buffer is full.
 * returns a pointer to a RNG struct if successful
 * 
 * This is a critical function,make sure to use the queue 
 * struct's mutex variable to lock pthread_mutex_lock()
 * before calling this function
 * after calling this,make sure to call pthead_mutex_lock()
 * on the RNG struct this function returns before modfiying it.
 * *********************************************************/

inline RNG *
enqueue (Q * q) {

  RNG *ret;

  if (q->in >= q->rsz)
    q->in = 0;	

  if ((q->sz>=(q->rsz-1)) || (q->in == (q->out - 1))) //Full
    return NULL;

  		// round and round go...    
  if (q->sz < q->rsz) {
    ret = &q->R[q->in++];
    //q->in++;
    q->sz++;
    return ret;
  }

  return NULL;
}

/*
 * q- handle to the ring buffer 
 * 
 * Returns NULL when the buffer is empty.
 * Returns a char*,the size of the char * is equivalent to
 * the third argument of init();
 * 
 
 * This is a critical function,make sure to use the queue 
 * struct's mutex variable to lock pthread_mutex_lock()
 * before calling this function
 * *************************************************/

inline RNG *
pop (Q * q) {
  RNG *ret = NULL;

  if (q->out >= q->rsz)
    q->out = 0;

  if ((q->sz < 1) )//|| (q->in == q->out-1) )
    return NULL;

  ret = &q->R[q->out++];
  //q->out++;
  q->sz--;
  return ret;
}
