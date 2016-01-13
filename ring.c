#include "ring.h"
#include <string.h>

/*
 *   q - handle to the circular ring buffer 'queue'
 *   size - the ring buffer's size in bytes
 *   chunks - block size,for network traffic this would be 'mtu'
 *            the size of the data units in the ring buffer.
 *   hdr - encapsulatio header struct - used to pre-intialize header
 * *******************************************************************/

void init_ring_buffer ( Q * q, int size, int chunks, struct headers *hdr )
{

    int i = 0, k = 0;
    uint8_t *datapool;

    uint8_t *hdr_eth = ( uint8_t * ) & hdr->eh;
    uint8_t *hdr_ip = ( uint8_t * ) & hdr->iph;

    q->bsz = chunks + sizeof ( uint32_t );
    q->rsz = ( size / q->bsz );
    q->R = calloc ( q->rsz, sizeof ( RNG ) ); //fyi... calloc 0's out memory

    datapool = calloc ( q->rsz+1, q->bsz );
    if ( q->R == NULL || datapool == NULL ) {
	die ( 1, "ERROR during memory allocation" );
    }

    for ( i = 0; i < q->rsz; i++ ) {

	q->R[i].header = HEADER;
	q->R[i].ethernet_frame = &datapool[i * q->bsz];
	q->R[i].ipheader = ( struct iphdr * ) &q->R[i].ethernet_frame[ETH_HDRLEN];
	q->R[i].data = ( uint8_t * ) & q->R[i].ethernet_frame[ETHIP4];
	q->R[i].index = i;
	q->R[i].footer = &datapool[( ( i + 1 ) * ( q->bsz ) ) - sizeof ( uint32_t )];
	memcpy ( q->R[i].footer, &FOOTER, sizeof ( uint32_t ) );
	for ( k = 0; k < ETH_HDRLEN; k++ ) {
	    q->R[i].ethernet_frame[k] = hdr_eth[k];
	}
	q->R[i].lock = 0;
	memcpy ( q->R[i].ipheader, &hdr->iph, sizeof ( struct iphdr ) );

    }
    
        q->lock = 0;
	for(i=0;i < q->rsz;i++){
	 if(!valid_RNG(&q->R[i])){
	  die(1,"Ring validation failed after initialization!! index %i\n",i); 
	 }
	}
}

inline int valid_RNG ( RNG * r )
{
    if ( r->header == HEADER && memcmp ( r->footer, &FOOTER, sizeof ( uint32_t ) ) == 0 )
	return 1;

    return 0;
}

/*
 
 * q - handle to the ring buffer 
 * 
 * returns NULL if the buffer is full or LOCKFAIL if lock can't be aquired.
 * returns a pointer to a RNG struct if successful.
 * the caller will need to adjust the queue size after manipulting it. 
 * the caller also needs to release the lock on the RNG data returned to it.
 **********************************************************/

inline RNG *push ( Q * q )
{
    RNG *ret = NULL;

    if (atomic_lock(&q->lock) ) {
	debug ( 5, "\033[1;92mPush %i can't get a lock returning LOCKFAIL\n" ,q->id);
	return LOCKFAIL;
    }
    while ( atomic_lock ( &q->R[q->in].lock ) ) {
	if ( q->in >= q->rsz )
	    q->in = 0;

	++q->in;
    }
    if ( q->in >= q->rsz )
	q->in = 0;
    if ( ( q->sz >= ( q->rsz-1 ) ) || ( q->in == ( q->out - 1 ) ) ) {	//Full
	atomic_unlock ( &q->R[q->in].lock );
	debug ( 4, "\033[1;92mPush %i - buffer full [%i %i] returning NULL\n",q->id,q->sz,q->rsz );
	atomic_unlock ( &q->lock );

	return NULL;
    }
    atomic_unlock ( &q->lock );
    return &q->R[q->in];
}

inline RNG *pop ( Q * q )
{
    int res, res2, res3;
    RNG *ret = NULL;

    if ( atomic_lock ( &q->lock ) ) {
	debug ( 6, "\033[1;93mPop %i can't get lock returning null \n", q->id );
	return LOCKFAIL;
    }
    if ( ( q->sz < 1 ) || ( q->in == q->out ) ) {
      q->sz=0;
	atomic_unlock ( &q->lock );
	debug ( 6, "\033[193mPop %i buffer empty returning null\n", q->id );
	return NULL;
    }
if ( q->out >= q->rsz-1 )
	    q->out = 0;
    while ( q->sz > 0 && atomic_lock ( &q->R[q->out].lock ) ) {
      debug(6,"Finding unlocked for %i , in %i out %i size %i rsz %i\n",
	       q->id, q->in , q->out , q->sz , q->rsz);
      q->out= q->out >= q->rsz-1 ? 0 : q->out+1;
    }

    atomic_unlock( &q->lock );
    return &q->R[q->out++];
}
