#include "workers.h"
#include "network.h"
#include <netinet/in.h>

void *
tx_io (void *a) {

  struct txq *h = (struct txq *) a;
  uint8_t *ether_frame, *realdata;
  int b, i, hdr_sz = (ETH_HDRLEN + IP4_HDRLEN);
  RNG *data;
  char type[30];

  switch (h->type) {
	  case TUN:
	    snprintf (type, 30, "TUN");
	    break;
	  case TAP:
	    snprintf (type, 30, "TAP");
	    break;
	  case UNIXSOCKET:
	    snprintf (type, 30, "UNIX-SOCKET");
	    break;
	  case PIPE:
	    snprintf (type, 30, "NAMED-PIPE");
	    break;
	  case IFMAP:
	    snprintf (type, 30, "INTERFACE-TO-INTERFACE MAP");
	    break;
	  default:
	    snprintf (type, 30, "UNKNOWN");
  }

  printf
    ("TX IO  Type: %s Local-interface: %s Remote Interface: %s Local-FD:%02x AF_PACKET-FD:%02x MTU: %i - ONLINE\r\n",
      type, h->tifname, h->ifname, h->fd, h->af, h->mtu);

  printf
    ("\r\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");


  while (1) {
    data = NULL;
    if (pthread_mutex_trylock (h->q->mutex) == 0) {
      data = enqueue (h->q);
      if (data == NULL) {
	h->lost_in++;
      }
      else {
	if(!valid_RNG(data)){
	  die(0,"Error validating RING");
	  print_tx_stats(h);
	}
// 		pthread_mutex_unlock (&data->rmut);

	if (pthread_mutex_lock (&data->rmut) == 0) {
	  pthread_mutex_unlock (h->q->mutex);
	  //do stuff here
	  data->len = 0;

	  data->len = read (h->fd, data->data, h->mtu - hdr_sz);


	  if (data->len == -1) {
	    die (0, "Error in tx_io interface %s\n", h->ifname);

	  }
	  else if(!valid_RNG(data)){
          die(0,"Error tx_io interface %s read corrupted ring!!",h->ifname);
	  }else{
	    data->ipheader->tot_len = data->len + IP4_HDRLEN;
	    data->ipheader->check =
	      csum ((unsigned short *) (data->data + sizeof (struct ethhdr)),
		    sizeof (struct iphdr) / 2);
////trace_dump("TX_IO - received",data);
	    h->packets_in++;
	    h->bytes_in += data->len;
	  }

	  pthread_mutex_unlock (&data->rmut);
	}
      }
      pthread_mutex_unlock (h->q->mutex);

    }
  }

}

void *
tx (void *a) {


  struct txq *h = (struct txq *) a;

  char type[30];
  int ret, i = 0, hdr_sz = sizeof (struct ethhdr) + sizeof (struct iphdr);

  RNG *data;

  switch (h->type) {
	  case TUN:
	    snprintf (type, 30, "TUN");
	    break;
	  case TAP:
	    snprintf (type, 30, "TAP");
	    break;
	  case UNIXSOCKET:
	    snprintf (type, 30, "UNIX-SOCKET");
	    break;
	  case PIPE:
	    snprintf (type, 30, "NAMED-PIPE");
	    break;
	  case IFMAP:
	    snprintf (type, 30, "INTERFACE-TO-INTERFACE MAP");
	    break;
	  default:
	    snprintf (type, 30, "UNKNOWN");
  }

  printf
    ("TX Worker  Type: %s Local-interface: %s Remote Interface: %s Local-FD:%02x AF_PACKET-FD:%02x MTU: %i - ONLINE\r\n",
     type, h->tifname, h->ifname, h->fd, h->af, h->mtu);
  printf
    ("\r\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");
  while (1) {
    if(show_stats)print_tx_stats(h);
    
    data = NULL;
    if (pthread_mutex_trylock (h->q->mutex) == 0) {
      data = pop (h->q);
      if (data == NULL) {
	h->lost_out++;

      }
      else {
// 	 pthread_mutex_unlock (&data->rmut);
	if (pthread_mutex_lock (&data->rmut) == 0) {
	  pthread_mutex_unlock (h->q->mutex);
	  if (data->len > 0)
	    ret =
	      sendto (h->af, data->ethernet_frame, data->len + (ETH_HDRLEN + IP4_HDRLEN), 0,
		      (struct sockaddr *) &h->sll, sizeof (h->sll));
	  if (ret < 1) {
	    die (0, "Error in sendto tx worker %s interface", h->ifname);
	  }else
	    if(!valid_RNG(data)){
		die(0,"Error in write() tx worker %s interface - Ring corrupted!!\n",h->ifname);
	      }
	  else {
	    h->packets_out++;
	    h->bytes_out += ret;
	    //trace_dump ("TX - Transmited", data);
	  }
	  pthread_mutex_unlock (&data->rmut);
	}
      }
      pthread_mutex_unlock (h->q->mutex);
    }

  }

}

void *
rx_io (void *a) {
  struct rxq *h = (struct rxq *) a;
  socklen_t sklen = sizeof (h->sll);
  int b, i;
  uint8_t dst[6];
  struct in_addr *p, *m, *src_ip, *dst_ip;

  p = (struct in_addr *) &h->peer_ip;
  m = (struct in_addr *) &h->my_ip;

  RNG *data;
  char type[30];

  memcpy (dst, h->sll.sll_addr, 6);
  switch (h->type) {
	  case TUN:
	    snprintf (type, 30, "TUN");
	    break;
	  case TAP:
	    snprintf (type, 30, "TAP");
	    break;
	  case UNIXSOCKET:
	    snprintf (type, 30, "UNIX-SOCKET");
	    break;
	  case PIPE:
	    snprintf (type, 30, "NAMED-PIPE");
	    break;
	  case IFMAP:
	    snprintf (type, 30, "INTERFACE-TO-INTERFACE MAP");
	    break;
	  default:
	    snprintf (type, 30, "UNKNOWN");
  }

  printf
    ("RX IO  Type: %s Local-interface: %s Remote Interface: %s Local-FD:%02x AF_PACKET-FD:%02x MTU: %i - ONLINE\r\n",
     type, h->tifname, h->ifname, h->fd, h->af, h->mtu);
  printf
    ("\r\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");

  while (1) {
    data = NULL;
    if (pthread_mutex_trylock (h->q->mutex) == 0) {
      data = enqueue (h->q);
      if (data == NULL) {
	h->lost_in++;
      }
      else {
if(!valid_RNG(data)){
	  die(0,"Error validating RING");
	  print_rx_stats(h);
	}
// 		pthread_mutex_unlock (&data->rmut);
	if (pthread_mutex_lock (&data->rmut) == 0) {
	  pthread_mutex_unlock (h->q->mutex);
	  //do stuff here
	  data->len = 0;

	  data->len =
	    recvfrom (h->af, data->ethernet_frame, h->mtu, 0, (struct sockaddr *) &h->sll, &sklen);


	  if (data->len == -1) {
	    die (0, "Error in rx_io interface %s\n", h->ifname);
	  }
	  else if (data->ipheader->saddr != h->peer_ip) {
	    data->len = -1;
//  printf("\r\n %x != %x\r\n",data->ipheader->saddr , h->peer_ip);
//   uint32_t   pip=h->peer_ip,mip=h->my_ip;
// char rpip[256],rmip[256],dpip[256],dmip[256];
// src_ip=(struct in_addr *) &data->ipheader->saddr;
// dst_ip=(struct in_addr *) &data->ipheader->daddr;
// strncpy(rpip,inet_ntoa(*(struct in_addr *) &pip),256);
// strncpy(rmip,inet_ntoa(*(struct in_addr *) &mip),256);
// strncpy(dpip,inet_ntoa(*src_ip),256);
// strncpy(dmip,inet_ntoa(*dst_ip),256);
//printf(">>> %s ------> %s | %s -----> %s\r\n", dpip,dmip,rpip,rmip );
	  }else 
	    if(!valid_RNG(data)){
          die(0,"Error rx_io interface %s read corrupted ring!!",h->ifname);
	  }else{
	    data->len -= (ETH_HDRLEN + IP4_HDRLEN);

	    ////trace_dump("RX_IO - received",data);

//   uint32_t   pip=h->peer_ip,mip=h->my_ip;
// char rpip[256],rmip[256],dpip[256],dmip[256];
// src_ip=(struct in_addr *) &data->ipheader->saddr;
// dst_ip=(struct in_addr *) &data->ipheader->daddr;
// strncpy(rpip,inet_ntoa(*(struct in_addr *) &pip),256);
// strncpy(rmip,inet_ntoa(*(struct in_addr *) &mip),256);
// strncpy(dpip,inet_ntoa(*src_ip),256);
// strncpy(dmip,inet_ntoa(*dst_ip),256);
// printf(">>> %s ------> %s | %s -----> %s\r\n", dpip,dmip,rpip,rmip );
	    h->packets_in++;
	    h->bytes_in += data->len;
	  }

	  pthread_mutex_unlock (&data->rmut);
	}
      }
      pthread_mutex_unlock (h->q->mutex);

    }
  }

}









void *
rx (void *a) {
  struct rxq *h = (struct rxq *) a;
  int ret, i = 0;
  RNG *data;
  char type[30];
  uint8_t dst[6];

  switch (h->type) {
	  case TUN:
	    snprintf (type, 30, "TUN");
	    break;
	  case TAP:
	    snprintf (type, 30, "TAP");
	    break;
	  case UNIXSOCKET:
	    snprintf (type, 30, "UNIX-SOCKET");
	    break;
	  case PIPE:
	    snprintf (type, 30, "NAMED-PIPE");
	    break;
	  case IFMAP:
	    snprintf (type, 30, "INTERFACE-TO-INTERFACE MAP");
	    break;
	  default:
	    snprintf (type, 30, "UNKNOWN");
  }

  printf
    ("RX Worker  Type: %s Local-interface: %s Remote Interface: %s Local-FD:%02x AF_PACKET-FD:%02x MTU: %i - ONLINE\r\n",
     type, h->tifname, h->ifname, h->fd, h->af, h->mtu);
  printf
    ("\r\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");
  while (1) {
    if(show_stats)print_rx_stats(h);
    data = NULL;
    if (pthread_mutex_trylock (h->q->mutex) == 0) {
      data = pop (h->q);
      if (data == NULL) {
	h->lost_out++;
      }
      else {
// 	pthread_mutex_unlock (&data->rmut);
	if (pthread_mutex_lock (&data->rmut) == 0) {
	  pthread_mutex_unlock (h->q->mutex);
	  if (data->len > 0) {

	    //trace_dump ("RX Transmitted", data);
	    ret = write (h->fd, data->ethernet_frame + (ETH_HDRLEN + IP4_HDRLEN), h->tmtu);	//, 0, (struct sockaddr *) &h->sll, sizeof (h->sll));
	    if (ret == -1) {
	      die (0, "Error in write() rx worker %s interface", h->tifname);
	      trace_dump ("RX Transmitted", data);
	    }else 
	      if(!valid_RNG(data)){
		die(0,"Error in write() rx workder %s interface - Ring corrupted!!\n",h->tifname);
	      }
	    else {
	      h->packets_out++;
	      h->bytes_out += ret;
	    }

	  }
	  pthread_mutex_unlock (&data->rmut);
	}
      }
      pthread_mutex_unlock (h->q->mutex);
    }

  }

}



void *
map_in (void *a) {
  struct mapq *h = (struct mapq *) a;


}

void *
map (void *a) {

  struct mapq *h = (struct mapq *) a;



}

void *
supervisor (void *a) {

  start_networking ();
  printf("\rThread count: Global %i , SUP: %i ,IO: %i ,WORKERS: %i",GTS.gtc,GTS.supc,GTS.ioc,GTS.lbrc);
  while (1) {
      show_stats=1; 
    sleep (interval/2);
      show_stats=0;
    sleep (interval/2);
  }
}
void print_tx_stats(struct txq *h){
  char type[32];
  
  
  switch (h->type) {
	  case TUN:
	    snprintf (type, 30, "TUN");
	    break;
	  case TAP:
	    snprintf (type, 30, "TAP");
	    break;
	  case UNIXSOCKET:
	    snprintf (type, 30, "UNIX-SOCKET");
	    break;
	  case PIPE:
	    snprintf (type, 30, "NAMED-PIPE");
	    break;
	  case IFMAP:
	    snprintf (type, 30, "INTERFACE-TO-INTERFACE MAP");
	    break;
	  default:
	    snprintf (type, 30, "UNKNOWN");
  }
printf("\r\n________________________________________________________________________________________________________\r\n");
printf("\033[1;32mINFO: Interface: %s Type: %s Tunnel: %s FD:%x AF:%x MTU:%i TMTU: %i \n"
        "STATS: LI:%i LO: %i  PI: %i PO: %i BI: %i BO %i TX LOSS:%i\n"
        "FIFO: BSZ: %i RSZ: %i SZ: %i HEAD:%i  TAIL:%i\n", 
	h->ifname,type,h->tifname,h->fd,h->af,h->mtu,h->tmtu,
       h->lost_in,h->lost_out,h->packets_in,h->packets_out,h->bytes_in,h->bytes_out,h->bytes_out-h->bytes_in,
       h->q->bsz,h->q->rsz,h->q->sz,h->q->in,h->q->out        
);
}

void print_rx_stats(struct rxq *h){
  char type[32];
  
  
  switch (h->type) {
	  case TUN:
	    snprintf (type, 30, "TUN");
	    break;
	  case TAP:
	    snprintf (type, 30, "TAP");
	    break;
	  case UNIXSOCKET:
	    snprintf (type, 30, "UNIX-SOCKET");
	    break;
	  case PIPE:
	    snprintf (type, 30, "NAMED-PIPE");
	    break;
	  case IFMAP:
	    snprintf (type, 30, "INTERFACE-TO-INTERFACE MAP");
	    break;
	  default:
	    snprintf (type, 30, "UNKNOWN");
  }
printf("\r\n________________________________________________________________________________________________________\r\n");
printf("\033[1;34mINFO: Interface: %s Type: %s Tunnel: %s FD:%x AF:%x MTU:%i TMTU: %i \n"
        "STATS: LI:%i LO: %i  PI: %i PO: %i BI: %i BO %i TX LOSS:%i\n"
        "FIFO: BSZ: %i RSZ: %i SZ: %i HEAD:%i  TAIL:%i\n", 
	h->ifname,type,h->tifname,h->fd,h->af,h->mtu,h->tmtu,
       h->lost_in,h->lost_out,h->packets_in,h->packets_out,h->bytes_in,h->bytes_out,h->bytes_out-h->bytes_in,
       h->q->bsz,h->q->rsz,h->q->sz,h->q->in,h->q->out        
);
}
void print_map_stats(struct mapq *h){
  char type[32];
  
  
  switch (h->type) {
	  case TUN:
	    snprintf (type, 30, "TUN");
	    break;
	  case TAP:
	    snprintf (type, 30, "TAP");
	    break;
	  case UNIXSOCKET:
	    snprintf (type, 30, "UNIX-SOCKET");
	    break;
	  case PIPE:
	    snprintf (type, 30, "NAMED-PIPE");
	    break;
	  case IFMAP:
	    snprintf (type, 30, "INTERFACE-TO-INTERFACE MAP");
	    break;
	  default:
	    snprintf (type, 30, "UNKNOWN");
  }
printf("\r\n________________________________________________________________________________________________________\r\n");
printf("INFO: Interface: %s Type: %s MAPIN: %s MAP:%x AF:%x MTU:%i  \n"
        "STATS: LI:%i LO: %i  PI: %i PO: %i BI: %i BO %i TX LOSS:%i\n"
        "FIFO: BSZ: %i RSZ: %i SZ: %i HEAD:%i  TAIL:%i\n", 
	h->ifname,type,h->mapifname,h->map,h->af,h->mtu,
       h->lost_in,h->lost_out,h->packets_in,h->packets_out,h->bytes_in,h->bytes_out,h->bytes_out-h->bytes_in,
       h->q->bsz,h->q->rsz,h->q->sz,h->q->in,h->q->out        
);
}