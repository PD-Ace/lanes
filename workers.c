#define _GNU_SOURCE
#ifndef __USE_GNU
#define __USE_GNU
#include <sched.h>
#endif

#include "workers.h"
#include "network.h"
#include <netinet/in.h>


void *tx_io ( void *a )
{
    struct txq *h = ( struct txq * ) a;
    struct pollfd pfd;
    uint8_t *ether_frame, *realdata;
    unsigned int b, i=0, j = 0, bytes, hdr_sz = ( ETH_HDRLEN + IP4_HDRLEN );
    RNG *data;
    char type[30];
    ++global.running;
    pfd.fd = h->fd;
    pfd.events = POLLIN;
    tun_set_queue(h->fd,1); //attach to tap queue

    switch ( h->type ) {
      case TUN:
	  snprintf ( type, 30, "TUN" );
	  break;
      case TAP:
	  snprintf ( type, 30, "TAP" );
	  break;
      case UNIXSOCKET:
	  snprintf ( type, 30, "UNIX-SOCKET" );
	  break;
      case PIPE:
	  snprintf ( type, 30, "NAMED-PIPE" );
	  break;
      case IFMAP:
	  snprintf ( type, 30, "INTERFACE-TO-INTERFACE MAP" );
	  break;
      default:
	  snprintf ( type, 30, "UNKNOWN" );
    }

    printf ( "TX IO  Type: %s Local-interface: %s Remote Interface: %s Local-FD:%02x AF_PACKET-FD:%02x MTU: %i - ONLINE\r\n", type, h->tifname, h->ifname, h->fd, h->af, h->mtu );

    printf ( "\r\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n" );
    for (data=NULL ;global.run==1 ; ) {
       i= i<h->qnum ? i+1 : 0;
	data = NULL;
	debug ( 5, "\033[1;33mTX %i Pushing\n",i );
	data = push ( &h->q[i] );
// 	for (i=0 ;  data == NULL || data==LOCKFAIL;i=0) {
// 	    for ( ; i < h->qnum && (data == NULL || data == LOCKFAIL); i++ ) {
// 		if ( h->q[i].sz < h->q[i].rsz )
// 		    data = push ( &h->q[i] );
// 		debug ( 5, "%i try current queue: %i\n", i );
// 	    }
// 	    
// 	}
	debug ( 5, "\033[1;33mTX %i Pushed\n",i );

	if ( data != NULL && data != LOCKFAIL) {
	    debug ( 5, "\033[1;33mTX %i Pushed not Null %i\n", i,data->index );
	    do{
	    poll ( &pfd, 1, -1 );
	    if ( pfd.revents & POLLIN )
		data->len = read ( h->fd, data->data, h->mtu - hdr_sz );

	    if ( data->len == -1 ) {
		die ( 0, "Error in tx_io interface %s\n", h->ifname );

	    } else {
		data->ipheader->tot_len = data->len + IP4_HDRLEN;
		data->ipheader->check = csum ( ( unsigned short * ) ( data->data + sizeof ( struct ethhdr ) ), sizeof ( struct iphdr ) / 2 );
		h->packets_in++;
		h->bytes_in += data->len;

	    }
	    }while(data->len==-1);
	    debug ( 5, "\033[1;33mTX %i Push releasing\n",i );
	    atomic_unlock ( &data->lock );
	     h->q[i].sz = h->q[i].sz>=h->q[i].rsz ? h->q[i].sz : h->q[i].sz+1;
	    __sync_fetch_and_add(&h->q[i].in,1);
 if(h->q[i].in>=h->q[i].rsz)
	       __sync_fetch_and_sub(&h->q[i].in,h->q[i].in);
	}
	
	
    }
    free_tx(h);
    debug(6,"TX IO Exited\n");
    --global.running;

}

void *tx_lb ( void *a )
{
    int i = 0, lowest = 0, tmp = 0;
    struct txq *h = ( struct txq * ) a;

    for ( ; 1; ) {

	for ( i = 1, lowest = 0, tmp = h->q[0].sz; i < h->qnum; i++ ) {
	    if ( h->q[i].sz < tmp ) {
		tmp = h->q[i].sz;
		lowest = i;
	    }
	    h->qnow = lowest;
	}
	nanosleep ( &h->lb_delay, NULL );
    }

}

void *tx ( void *a )
{
   
    struct txq *h = ( struct txq * ) a;
    struct headers hdr;
    int ret, test, hdr_sz = sizeof ( struct ethhdr ) + sizeof ( struct iphdr );
    unsigned int i= __sync_fetch_and_add ( &h->id, 1 ),c=0;
    char type[30];
    RNG *data;
    
    ++global.running;
    
    
    h->q[i].ts.tv_sec=0;
    h->q[i].ts.tv_nsec=SLOWSPIN;
    h->q[i].spins=0;
    h->q[i].id = i;
 
   
    
   
    //Pin this thread to a cpu 
   pthread_t this = pthread_self();       
   cpu_set_t cpuset;
   CPU_ZERO(&cpuset);   
   c= __sync_fetch_and_sub(&global.cpu_available,1);
   if(c>=0){ // if threads > cpu count , we won't pin any more threads
   CPU_SET(c, &cpuset);
   pthread_setaffinity_np(this, sizeof(cpu_set_t), &cpuset);
   } 
   
   
    int af=h->af;
   // set_fanout(0,af);
    switch ( h->type ) {
      case TUN:
	  snprintf ( type, 30, "TUN" );
	  break;
      case TAP:
	  snprintf ( type, 30, "TAP" );
	  break;
      case UNIXSOCKET:
	  snprintf ( type, 30, "UNIX-SOCKET" );
	  break;
      case PIPE:
	  snprintf ( type, 30, "NAMED-PIPE" );
	  break;
      case IFMAP:
	  snprintf ( type, 30, "INTERFACE-TO-INTERFACE MAP" );
	  break;
      default:
	  snprintf ( type, 30, "UNKNOWN" );
    }
    printf ( "TX Worker ID: %i Type: %s Local-interface: %s Remote Interface: %s Local-FD:%02x AF_PACKET-FD:%02x MTU: %i - ONLINE\r\n", i, type, h->tifname, h->ifname, h->fd, h->af, h->mtu );
    printf ( "\r\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n" );

    for (data=NULL ;global.run==1 ; ) {


	debug ( 5, "\033[1;33mTX %i { Popping}\n", i );

	data = pop ( &h->q[i] );
	debug ( 5, "\033[1;33mTX %i { Popped}\n", i );

	if ( data != NULL && data != LOCKFAIL) {
	    debug ( 5, "\033[1;33mTX %i {Popping Not null %i}\n", i, data->index );

	    if ( data->len > 14 ) {
		debug ( 5, "\033[1;33mTX %i {Popping has data %i}\n", i, data->index );
		ret = sendto ( af, data->ethernet_frame, data->len + ( ETH_HDRLEN + IP4_HDRLEN ), 0, ( struct sockaddr * ) &h->sll, sizeof ( h->sll ) );

		if ( ret < 1 ) {
		    die ( 0, "Error in sendto tx worker %s interface", h->ifname );
		} else {
		    h->packets_out++;
		    h->bytes_out += ret;
		    //	      trace_dump("TX out",data);

		}
	    } else {
		debug ( 5, "\033[1;33mTX %i { Popping no data %i}\n", i, data->index );

	    }
	    debug ( 5, "\033[1;33mTX %i {Pop releasing %i}\n", i, data->index );
	    data->len = -1;
	    	     h->q[i].sz = h->q[i].sz < 1 ? 0 : h->q[i].sz-1;

	    atomic_unlock ( &data->lock );
	    
	} else if(data == NULL) {

	    debug ( 5, "\033[1;33mTX %i {Entering size wait - Got Null}\n", i );
	    if(h->q[i].ts.tv_nsec >= (SLOWSPIN*4))
	        h->q[i].ts.tv_nsec = SLOWSPIN;
	    else h->q[i].ts.tv_nsec += (SLOWSPIN/10);
	    
	    while ( h->q[i].sz==0 )nanosleep(&h->q[i].ts,NULL);
	    debug ( 5, "\033[1;33mTX %i {Size change resuming  size %i}\n", i, h->q[i].sz );
	}else if(data == LOCKFAIL){
	 debug(5, "\033[1;33mTX %i LOCKFAIL. size %i in %i out %i\n",
	              i,h->q[i].sz,h->q[i].in,h->q[i].out);
	}
    }
    debug(6,"TX Worker %i Exited\n",i);
    --global.running;

}

void *rx_io ( void *a )
{
    struct rxq *h = ( struct rxq * ) a;
    struct pollfd pfd;
    socklen_t sklen = sizeof ( h->sll );
    int bytes;
    unsigned int i=0, j = 0,nope=0;
    uint8_t dst[6];
    struct in_addr *p, *m, *src_ip, *dst_ip;
    ++global.running;
    p = ( struct in_addr * ) &h->peer_ip;
    m = ( struct in_addr * ) &h->my_ip;

    RNG *data;
    char type[30];

    memcpy ( dst, h->sll.sll_addr, 6 );

    pfd.fd = h->af;
    pfd.events = POLLIN;
    int af=h->af;
   // set_fanout(0,af);
    switch ( h->type ) {
      case TUN:
	  snprintf ( type, 30, "TUN" );
	  break;
      case TAP:
	  snprintf ( type, 30, "TAP" );
	  break;
      case UNIXSOCKET:
	  snprintf ( type, 30, "UNIX-SOCKET" );
	  break;
      case PIPE:
	  snprintf ( type, 30, "NAMED-PIPE" );
	  break;
      case IFMAP:
	  snprintf ( type, 30, "INTERFACE-TO-INTERFACE MAP" );
	  break;
      default:
	  snprintf ( type, 30, "UNKNOWN" );
    }

    printf ( "RX IO Type: %s Local-interface: %s Remote Interface: %s Local-FD:%02x AF_PACKET-FD:%02x MTU: %i - ONLINE\r\n", type, h->tifname, h->ifname, h->fd, h->af, h->mtu );
    printf ( "\r\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n" );

    for (data=NULL ;global.run==1 ; i= i<h->qnum ? i+1 : 0 ) {
      
	debug ( 6, "\033[1;31mRX %i Pushing\n", i );
	data = push ( &h->q[i] );
// 	for (i=0 ; data == NULL || data == LOCKFAIL; i=0) {
// 	    for ( ; i < h->qnum && (data == NULL || data==LOCKFAIL); ++i ) {
// 		if ( h->q[i].sz < h->q[i].rsz ){
// 		    data = push ( &h->q[i] );
// 		debug ( 5, "%i try current queue: %i\n", j, i );
// 		}
// 	    }
// 	}
	debug ( 6, "\033[1;31mRX %i Pushed\n", i );
	if ( data != NULL && data != LOCKFAIL) {
	    debug ( 6, "\033[1;31mRX %i Pushed not null index %i\n", i, data->index );
	    do{
	    poll ( &pfd, 1, -1 );
	    if ( pfd.revents & POLLIN )
		data->len = read ( af, data->ethernet_frame, h->mtu );	//, NULL, 0);

	    if ( data->len < 1) {
		die ( 0, "Error in rx_io interface %s\n", h->ifname );
		nope=1;
	    } else if ( data->ipheader->saddr != h->peer_ip ) {
	      nope=1;
// 		data->len = -1;
// 		__sync_val_compare_and_swap ( &data->lock, 1, 0 );
        	//debug ( 6, "\033[1;31mRX %i Push releasing Non-matching IP \n", i );
	              //     printf("!");
	      

	    }else{
	      nope=0;
	    	      //trace_dump("RX io",data);
	    }
	    }while(nope);
	    
		data->len -= ETHIP4;
		debug ( 6, "\033[1;31mRX %i Got matching IP index %i \n", i,data->index );
	    

	    debug ( 6, "\033[1;31mRX %i Push releasing %i\n", i,data->index );
	    atomic_unlock ( &data->lock );
            h->q[i].sz = h->q[i].sz>=h->q[i].rsz ? h->q[i].sz : h->q[i].sz+1;
             __sync_fetch_and_add(&h->q[i].in,1);
             if(h->q[i].in>=h->q[i].rsz)
	       __sync_fetch_and_sub(&h->q[i].in,h->q[i].in);
	}

    }
    free_rx(h);
    debug(6,"RX IO Exited\n");
    --global.running;

}

void *rx_lb ( void *a )
{
    int i = 0, lowest = 0, tmp = 0;
    struct rxq *h = ( struct rxq * ) a;

    for ( ; 1; ) {

	for ( i = 1, lowest = 0, tmp = h->q[0].sz; i < h->qnum; i++ ) {
	    if ( h->q[i].sz < tmp ) {
		tmp = h->q[i].sz;
		lowest = i;
	    }
	    h->qnow = lowest;
	}
	nanosleep ( &h->lb_delay, NULL );
    }

}

void *rx ( void *a )
{
    struct rxq *h = ( struct rxq * ) a;

    int ret, i;
    RNG *data;
    char type[30];
    uint8_t dst[6];
    struct timespec ts;
    struct headers hdr;
    pthread_t this = pthread_self();    
    ++global.running;
   cpu_set_t cpuset;
   CPU_ZERO(&cpuset);
   
   i= __sync_fetch_and_sub(&global.cpu_available,1);
  
   if(i>=0){ // if threads > cpu count , we won't pin any more threads
   CPU_SET(i, &cpuset);
   pthread_setaffinity_np(this, sizeof(cpu_set_t), &cpuset);
   }
    i = __sync_fetch_and_add ( &h->id, 1 );
    h->q[i].id = i;
    ts.tv_sec = 0;
    ts.tv_nsec = SLOWSPIN;
    tun_set_queue(h->fd,1); //attach to tap queue
    
    switch ( h->type ) {
      case TUN:
	  snprintf ( type, 30, "TUN" );
	  break;
      case TAP:
	  snprintf ( type, 30, "TAP" );
	  break;
      case UNIXSOCKET:
	  snprintf ( type, 30, "UNIX-SOCKET" );
	  break;
      case PIPE:
	  snprintf ( type, 30, "NAMED-PIPE" );
	  break;
      case IFMAP:
	  snprintf ( type, 30, "INTERFACE-TO-INTERFACE MAP" );
	  break;
      default:
	  snprintf ( type, 30, "UNKNOWN" );
    }

    printf ( "RX Worker ID: %i Type: %s Local-interface: %s Remote Interface: %s Local-FD:%02x AF_PACKET-FD:%02x MTU: %i - ONLINE\r\n", i, type, h->tifname, h->ifname, h->fd, h->af, h->mtu );
    printf ( "\r\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n" );

    for (data=NULL ;global.run==1 ; ) {
	

	debug ( 6, "\033[1;31mRX %i Popping\n", i );
	data = pop ( &h->q[i] );
	debug ( 6, "\033[1;31mRX %i Popped\n", i );

	if ( data != NULL && data != LOCKFAIL ) {
	    debug ( 6, "\033[1;31mRX %i Popped Not null\n", i );

	    if ( data->len > 14 ) {
		debug ( 6, "\033[1;31mRX %i Popped has data %i got %i\n", i, data->index, data->len );

		ret = write ( h->fd, data->ethernet_frame + ( ETH_HDRLEN + IP4_HDRLEN ), data->len );	//, 0, (struct sockaddr *) &h->sll, sizeof (h->sll));
		if ( ret < 1 ) {
		    die ( 0, "Error in write() rx worker %s interface", h->tifname );
		    //		trace_dump("Invalid write!",data);

		} else {
		    h->packets_out++;
		    h->bytes_out += ret;

		    debug ( 6, "\033[1;31mRX %i wrote %i\n", i, data->index );

		}
		//data->len = -1;
	    }
	    else {
		debug ( 6, "\033[1;31mRX %i got no data %i\n", i, data->index );
	    }
	    debug ( 6, "\033[1;31mRX %i releasing %i\n", i, data->index );
	    		data->len = -1;
	    h->q[i].sz = h->q[i].sz < 1 ? 0 : h->q[i].sz-1;
	    atomic_unlock ( &data->lock );
	} else if (data==NULL){
	    debug ( 6, "\033[1;31mRX %i Entering size wait - Got Null\n", i );
	    if(h->q[i].ts.tv_nsec >= (SLOWSPIN*4))
	        h->q[i].ts.tv_nsec = SLOWSPIN;
	    else h->q[i].ts.tv_nsec += (SLOWSPIN/10);
	    
	    while ( h->q[i].sz < 1 )nanosleep(&h->q[i].ts,NULL);
	    debug ( 6, "\033[1;31mRX %i Size change resuming size %i\n", i, h->q[i].sz );
	}else if  ( data==LOCKFAIL){
	 debug(6, "\033[1;33mRX %i LOCKFAIL. size %i in %i out %i\n",
	              i,h->q[i].sz,h->q[i].in,h->q[i].out); 
	}

    }
    debug(6,"RX Worker %i Exited\n",i);
    --global.running;

}

void *map_in ( void *a )
{
    struct mapq **ph = ( struct mapq ** ) a;

}

void *map_lb ( void *a )
{

}

void *map ( void *a )
{

    struct mapq **ph = ( struct mapq ** ) a;

}

void *supervisor ( void *a )
{

    start_networking (  );
    sleep(5);
    drop_privs();
    debug(4,"Priviledge dropped,SECCOMP loaded.\n");
     
    printf ( "\rThread count: Global %i , SUP: %i ,IO: %i ,WORKERS: %i\n", GTS.gtc, GTS.supc, GTS.ioc, GTS.lbrc );
    while ( global.run ) {
	show_stats = 1;
	sleep ( interval / 2 );
	show_stats = 0;
	sleep ( interval / 2 );
    }
}

void print_tx_stats ( struct txq *h )
{
    char type[32];

    switch ( h->type ) {
      case TUN:
	  snprintf ( type, 30, "TUN" );
	  break;
      case TAP:
	  snprintf ( type, 30, "TAP" );
	  break;
      case UNIXSOCKET:
	  snprintf ( type, 30, "UNIX-SOCKET" );
	  break;
      case PIPE:
	  snprintf ( type, 30, "NAMED-PIPE" );
	  break;
      case IFMAP:
	  snprintf ( type, 30, "INTERFACE-TO-INTERFACE MAP" );
	  break;
      default:
	  snprintf ( type, 30, "UNKNOWN" );
    }
// printf("\r\n________________________________________________________________________________________________________\r\n");
// printf("\033[1;32mINFO: Interface: %s Type: %s Tunnel: %s FD:%x AF:%x MTU:%i TMTU: %i \n"
//         "STATS: TLI:%i TLO: %i  PI: %i PO: %i BI: %i BO %i TX LOSS:%i\n"
//        // "FIFO: BSZ: %i RSZ: %i SZ: %i HEAD:%i  TAIL:%i\n", 
//      h->ifname,type,h->tifname,h->fd,h->af,h->mtu,h->tmtu,
//        h->lost_in_tx,h->lost_out_tx,h->packets_in,h->packets_out,h->bytes_in,h->bytes_out,h->bytes_out-h->bytes_in
//     //   q->bsz,q->rsz,q->sz,q->in,q->out        
// );
}

void print_rx_stats ( struct rxq *h )
{
    char type[32];

    switch ( h->type ) {
      case TUN:
	  snprintf ( type, 30, "TUN" );
	  break;
      case TAP:
	  snprintf ( type, 30, "TAP" );
	  break;
      case UNIXSOCKET:
	  snprintf ( type, 30, "UNIX-SOCKET" );
	  break;
      case PIPE:
	  snprintf ( type, 30, "NAMED-PIPE" );
	  break;
      case IFMAP:
	  snprintf ( type, 30, "INTERFACE-TO-INTERFACE MAP" );
	  break;
      default:
	  snprintf ( type, 30, "UNKNOWN" );
    }
// printf("\r\n________________________________________________________________________________________________________\r\n");
// printf("\033[1;34mINFO: Interface: %s Type: %s Tunnel: %s FD:%x AF:%x MTU:%i TMTU: %i \n"
//         "STATS: RLI:%i RLO: %i  PI: %i PO: %i BI: %i BO %i TX LOSS:%i\n"
//   //      "FIFO: BSZ: %i RSZ: %i SZ: %i HEAD:%i  TAIL:%i\n", 
//      h->ifname,type,h->tifname,h->fd,h->af,h->mtu,h->tmtu,
//        h->lost_in_rx,h->lost_out_rx,h->packets_in,h->packets_out,h->bytes_in,h->bytes_out,h->bytes_out-h->bytes_in
//    //    q->bsz,q->rsz,q->sz,q->in,q->out        
// );
}

void print_map_stats ( struct mapq *h )
{
    char type[32];

    switch ( h->type ) {
      case TUN:
	  snprintf ( type, 30, "TUN" );
	  break;
      case TAP:
	  snprintf ( type, 30, "TAP" );
	  break;
      case UNIXSOCKET:
	  snprintf ( type, 30, "UNIX-SOCKET" );
	  break;
      case PIPE:
	  snprintf ( type, 30, "NAMED-PIPE" );
	  break;
      case IFMAP:
	  snprintf ( type, 30, "INTERFACE-TO-INTERFACE MAP" );
	  break;
      default:
	  snprintf ( type, 30, "UNKNOWN" );
    }
// printf("\r\n________________________________________________________________________________________________________\r\n");
// printf("INFO: Interface: %s Type: %s MAPIN: %s MAP:%x AF:%x MTU:%i  \n"
//         "STATS: LI:%i LO: %i  PI: %i PO: %i BI: %i BO %i TX LOSS:%i\n"
//     //    "FIFO: BSZ: %i RSZ: %i SZ: %i HEAD:%i  TAIL:%i\n", 
//      h->ifname,type,h->mapifname,h->map,h->af,h->mtu,
//        h->lost_in,h->lost_out,h->packets_in,h->packets_out,h->bytes_in,h->bytes_out,h->bytes_out-h->bytes_in
//     //   q->bsz,q->rsz,q->sz,q->in,q->out        
// );
}
