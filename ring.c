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
    q->R = calloc ( q->rsz, sizeof ( RNG ) );

    datapool = calloc ( q->rsz, q->bsz );
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
 * returns NULL if the buffer is full or lock can't be aquired.
 * returns a pointer to a RNG struct if successful.
 * the caller will need to adjust the queue size after manipulting it. 
 * the caller also needs to release the lock on the RNG data returned to it.
 **********************************************************/

inline RNG *push ( Q * q )
{
    RNG *ret = NULL;

    if ( __sync_val_compare_and_swap ( &q->lock, 0, 1 ) ) {
	debug ( 5, "Push %i can't get a lock returning NULL\n" ,q->id);
	return NULL;
    }
    while ( __sync_val_compare_and_swap ( &q->R[q->in].lock, 0, 1 ) ) {
	if ( q->in >= q->rsz )
	    q->in = 0;

	++q->in;
    }
    if ( q->in >= q->rsz )
	q->in = 0;
    if ( ( q->sz >= ( q->rsz ) ) || ( q->in == ( q->out - 1 ) ) ) {	//Full
	__sync_val_compare_and_swap ( &q->R[q->in].lock, 1, 0 );
	debug ( 4, "Push %i - buffer full [%i -i] returning NULL\n",q->id,q->sz,q->rsz );
	__sync_val_compare_and_swap ( &q->lock, 1, 0 );

	return NULL;
    }
    __sync_val_compare_and_swap ( &q->lock, 1, 0 );
    return &q->R[q->in];
}

inline RNG *pop ( Q * q )
{
    int res, res2, res3;
    RNG *ret = NULL;

    if ( __sync_val_compare_and_swap ( &q->lock, 0, 1 ) ) {
	debug ( 6, "Pop %i can't get lock returning null \n", q->id );
	return NULL;
    }
    if ( ( q->sz < 1 ) || ( q->in == q->out ) ) {
	__sync_val_compare_and_swap ( &q->lock, 1, 0 );
	debug ( 6, "Pop %i buffer empty returning null\n", q->id );
	return NULL;
    }

    while ( __sync_val_compare_and_swap ( &q->R[q->out].lock, 0, 1 ) ) {
	if ( q->out >= q->rsz )
	    q->out = 0;
	++q->out;
    }
    		        q->sz--;

    __sync_val_compare_and_swap ( &q->lock, 1, 0 );
    return &q->R[q->out++];
}
