#include "network.h"
#include "util.h"

void
get_interface (char *if_name, struct ifreq *ifr, int d) {

  size_t if_name_len = strlen (if_name);

  if (if_name_len < sizeof (ifr->ifr_name)) {
    memcpy (ifr->ifr_name, if_name, if_name_len);
    ifr->ifr_name[if_name_len] = 0;
  }
  else {
    die (0, "interface name is too long");
  }
  int fd = socket (AF_INET, SOCK_DGRAM, 0);

  if (fd == -1) {
    die (0, if_name);
  }
  else {
    switch (d) {

	    case IFINDEX:
	      if (ioctl (fd, SIOCGIFINDEX, ifr) == -1) {
		die (0, "IFINDEX");
	      }
	      break;
	    case IFMAC:
	      if (ioctl (fd, SIOCGIFHWADDR, ifr) == -1) {
		die (0, "IFMAC");
	      }
	      break;
	    case IFMTU:
	      if (ioctl (fd, SIOCGIFMTU, ifr) == -1) {
		die (0, "IFMTU");
	      }
	      break;
	    case IFADDR:
	      if (ioctl (fd, SIOCGIFADDR, ifr) == -1) {
		// die(0,"IFADDR");
	      }
	      break;
	    default:
	      break;
    }
    close (fd);
  }
}


int
ifup (char *if_name) {
  struct ifreq ifr;
  size_t if_name_len = strlen (if_name);

  if (if_name_len < sizeof (ifr.ifr_name)) {
    memcpy (ifr.ifr_name, if_name, if_name_len);
    ifr.ifr_name[if_name_len] = 0;
  }
  else {
    die (0, "interface name is too long");
  }
  int fd = socket (AF_UNIX, SOCK_DGRAM, 0);

  if (fd == -1) {
    die (0, if_name);
  }
  if (ioctl (fd, SIOCGIFFLAGS, &ifr) == -1) {
    die (0, if_name);
  }
  else {
    int flag = ifr.ifr_flags;

    if (flag & IFF_UP) {
      printf ("%s is UP\r\n", if_name);
    }
    else if (!(flag & IFF_UP)) {
      printf ("%s is DOWN. bringing interface UP\r\n", if_name);
      ifr.ifr_flags |= IFF_UP;

      if (ioctl (fd, SIOCSIFFLAGS, &ifr) == -1) {
	die (0, "Interface turn up");
	return 1;
      }
      else {
	printf ("Successfully brought %s back online\r\n", if_name);
      }
    }
  }

  return 0;
}




int
enumerate_interfaces () {
  int fd = -1, index, mtu = 1500, txc = 0, rxc = 0, mpc = 0, i = 0, j = 0, k = 0, s =
    -1, sz, local = 0;
  char ipstring[64];
  struct sockaddr_un uxs;
  struct sockaddr_in *ipaddr;
 const unsigned char *mac;
  struct list_head *lh;
  struct peer_context *tmp;

  struct ifreq ifr;
  struct headers hdr;


  list_for_each_prev (lh, &(MPL.PL)) {
    tmp = list_entry (lh, struct peer_context, PL);

    if (tmp != NULL) {
      printf ("\r\n+-----------------------------------------------------------------------+\r\n");
      if (!ifup (tmp->ifname)) {
	rxc++;
      }
      if (strlen (tmp->ifmap) > 0) {
	if (!ifup (tmp->ifmap)) {
	  mpc++;
	}
      }

      get_interface (tmp->ifname, &ifr, IFINDEX);
      index = ifr.ifr_ifindex;
      get_interface (tmp->ifname, &ifr, IFMTU);
      mtu = ifr.ifr_mtu;

      get_interface (tmp->ifname, &ifr, IFADDR);

      ipaddr = (struct sockaddr_in *) &ifr.ifr_addr;

      get_interface (tmp->ifname, &ifr, IFMAC);
      mac = (unsigned char *) ifr.ifr_hwaddr.sa_data;
      if (strlen (tmp->tifname) > 0 || strlen (tmp->socket_path) > 0) {
	txc++;
      }


      printf
	("%s :: Tunnel[%s] Interface [%u{%s}], Mac: %02X:%02X:%02X:%02X:%02X:%02X IP: %s MTU:%i \r\n",
	 tmp->name, tmp->tifname, index, tmp->ifname, mac[0], mac[1], mac[2], mac[3], mac[4],
	 mac[5], inet_ntoa (ipaddr->sin_addr), mtu);
      fflush (stdout);



    }

  }

/////////////////RX TX MAP SETUP/////////////

  
  TX = malloc ((txc * sizeof (struct txq)));
  RX = malloc ((rxc * sizeof (struct rxq)));
  M = malloc ((mpc * sizeof (struct mapq)));
  
  txc = 0;
  rxc = 0;
  mpc = 0;
  list_for_each_prev (lh, &(MPL.PL)) {

  
  tmp = list_entry (lh, struct peer_context, PL);

    if (tmp->direction == OUT) {	// egress /outgoing
tx_session(&TX,txc++,rxc,tmp,&RX);


    }
    else if (tmp->direction == IN) {	// Ingress incoming
 rx_session(&RX,rxc++,txc,tmp,&TX);


      
    }
    else if (tmp->direction == MAP) {


      new_thread (IO, &map_in, (void *) &M);
      for (j = 0; j < tmp->threads; j++) {
	new_thread (IO, &map, (void *) &M);
      }
    }
  }

  return 1;
}

void tx_session(struct txq **TX,int txc,int rxc,struct peer_context *tmp,struct rxq **RX){
         struct ifreq ifr;
	 struct txq *txs= TX[txc];
         struct thread_management *tmtmp;
 	 struct headers hdr;

	 int k,fd,j;
	 const unsigned char *mac;
       if(txs==NULL || RX==NULL || tmp==NULL){
	 die(1,"Error setting up tx session !!");
       }
       
      if (strlen (tmp->ifname) > 0) {

	txs->af = init_af_packet (tmp->ifname, &txs->sll);

	if (txs->af > -1) {
	  get_interface (tmp->ifname, &ifr, IFMTU);
	  txs->mtu = ifr.ifr_mtu;

	  if (strlen (tmp->tifname) > 0) {


	    for (k = 0; k < txc; k++) {
	      if (TX[k]->fd > 0 && strncmp (TX[k]->tifname, tmp->tifname, IFNAMSIZ) == 0) {
		fd = TX[k]->fd;
	      }
	    }
	    for (k = 0; k < rxc; k++) {
	      if (RX[k]->fd > 0 && strncmp (RX[k]->tifname, tmp->tifname, IFNAMSIZ) == 0) {
		fd = RX[k]->fd;
	      }
	    }
	    
	    if (fd < 1)
	      fd = tuntap (tmp->tifname, IFF_TAP | IFF_NO_PI| IFF_MULTI_QUEUE);
	    ifup (tmp->tifname);
	    if (fd > 0) {
	      printf ("TUN %s is ready - %i\r\n", tmp->tifname, fd);
	      txs->type = TUN;
	      txs->fd = fd;
	      memcpy (txs->tifname, tmp->tifname, IFNAMSIZ);
	      memcpy (txs->ifname, tmp->ifname, IFNAMSIZ);
	      get_interface (tmp->ifname, &ifr, IFMAC);
	      mac = (unsigned char *) ifr.ifr_hwaddr.sa_data;
	      memcpy (txs->sll.sll_addr, mac, 6 * sizeof (uint8_t));

	      get_interface (tmp->ifname, &ifr, IFINDEX);
	      txs->sll.sll_ifindex = ifr.ifr_ifindex;;
	      txs->sll.sll_halen = 6;
	      txs->sll.sll_protocol = htons (ETH_P_ALL);
      	      txs->sll.sll_family = AF_PACKET;

	      set_ip (tmp->tifname, tmp->myip);
	      txs->peer_ip = tmp->peerip;
	      txs->my_ip = tmp->myip;

	      set_mtu (tmp->tifname, txs->mtu-(ETH_HDRLEN + IP4_HDRLEN));	//txs->mtu-sizeof(struct headers));
              txs->pcx=tmp; 
	      txs->qnum=tmp->threads;
	      txs->lb_delay.tv_nsec=LBDELAY;
             txs->q = malloc (sizeof (Q) * tmp->threads);

	         fill_headers (txs->pcx, &hdr, txs->af);

	      
	      
	      for (j = 0; j < tmp->threads; j++) {
		
		//txs->q[j].R = calloc (BUFSIZE/(txs->mtu+sizeof(uint32_t)),(txs->mtu+sizeof(uint32_t)));
		      init_ring_buffer (&txs->q[j],BUFSIZE/tmp->threads, txs->mtu, &hdr);
                     txs->q[j].id=j;
		new_thread (LBR, &tx, (void *) txs);
		
	      }
	      	      new_thread (IO, &tx_io, (void *) txs);
       	      	    //  new_thread (IO, &tx_lb, (void *) txs);

//                          local=1;
	    }
	    else {
	      printf ("TUNTAP %s for %s not ready", tmp->tifname, tmp->name);
	      die (0, "TUNTAP error %s for %s not ready", tmp->tifname, tmp->name);
	    }
	  }
	  else if (strlen (tmp->socket_path) > 0) {

	    /* txs->type=UNIXSOCKET;
	       s = socket(AF_UNIX, SOCK_STREAM, 0);
	       if (s == -1) {
	       die(1,"Error creating unix socket");

	       }
	       uxs.sun_family= AF_UNIX;
	       strncpy(uxs.sun_path,tmp->socket_path,255);
	       unlink(uxs.sun_path);
	       sz=strlen(uxs.sun_path)+sizeof(uxs.sun_family);  
	       if (connect(s, (struct sockaddr *)&uxs, sz) == -1) {
	       die(1,"Error binding unix socket %s",uxs.sun_path);
	       }else{
	       printf("Unix socket %s is ready\r\n",tmp->socket_path);
	       txs->fd=s; 
	       } */
	    //   init_ring_buffer((Q*)txtmp.q,BUFSIZE,txs->mtu);
	  }
	  else if (strlen (tmp->pipe_path) > 0) {
	    int tmpfd = open (tmp->pipe_path, O_RDONLY | O_CREAT);

	    if (tmpfd < 0) {
	      die (1, "ERROR - %s - Unable to open named pipe %s for data source -", tmp->name,
		   tmp->pipe_path);
	      die (1,
		   "ERROR - No valid local data source specified for an Egress(out) tunnel %s\r\n",
		   tmp->name);
	    }
	    else {
	      txs->type = PIPE;
	      txs->fd = tmpfd;
	    }

	  }
	  else {
	    die (1,
		 "ERROR - No valid local data source specified for an Egress(out) tunnel for %s \r\n",
		 tmp->name);
	  }
	}
	else {
	  die (1, "ERROR -%s- Unable to open raw socket for %s", tmp->name, tmp->ifname);
	}
      }
      else {
	die (1, "ERROR - No valid Egress(out) interface specified for an Egress tunnel for %s",
	     tmp->name);
      }
 
}
void rx_session(struct rxq **RX,int rxc,int txc,struct peer_context *tmp,struct txq **TX){
           struct ifreq ifr;
	 struct rxq *rxs= RX[rxc];
	 struct headers hdr;
	   
	 int k,fd,j;
	 const unsigned char *mac;
       if(rxs==NULL || TX==NULL || tmp==NULL){
	 die(1,"Error setting up tx session !!");
       }
       
        if (strlen (tmp->ifname) > 0) {

	rxs->af = init_af_packet (tmp->ifname, &rxs->sll);

	if (rxs->af > -1) {
	  get_interface (tmp->ifname, &ifr, IFMTU);
	  rxs->mtu = ifr.ifr_mtu;

	  if (strlen (tmp->tifname) > 0) {


	    for (k = 0; k < rxc; k++) {
	      if (RX[k]->fd > 0 && strncmp (RX[k]->tifname, tmp->tifname, IFNAMSIZ) == 0) {
		fd = RX[k]->fd;
	      }
	    }

	    for (k = 0; k < txc; k++) {
	      if (TX[k]->fd > 0 && strncmp (TX[k]->tifname, tmp->tifname, IFNAMSIZ) == 0) {
		fd = TX[k]->fd;
	      }
	    }
	    if (fd < 1)
	      fd = tuntap (tmp->tifname, IFF_TAP | IFF_NO_PI| IFF_MULTI_QUEUE);
	    ifup (tmp->tifname);
	    if (fd > 0) {
	      printf ("TUN %s is ready - %i\r\n", tmp->tifname, fd);
	      rxs->type = TUN;
	      rxs->fd = fd;
	      memcpy (rxs->tifname, tmp->tifname, IFNAMSIZ);
	      memcpy (rxs->ifname, tmp->ifname, IFNAMSIZ);

	      get_interface (tmp->tifname, &ifr, IFMAC);
	      mac = (unsigned char *) ifr.ifr_hwaddr.sa_data;
	      memcpy (rxs->sll.sll_addr, mac, 6 * sizeof (uint8_t));

	      rxs->sll.sll_family = AF_PACKET;
	      get_interface (tmp->ifname, &ifr, IFINDEX);
	      rxs->sll.sll_ifindex = ifr.ifr_ifindex;;
	      rxs->sll.sll_halen = 6;
	      rxs->sll.sll_protocol = htons (ETH_P_ALL);
              rxs->tmtu=rxs->mtu-(ETH_HDRLEN + IP4_HDRLEN);
	      //rxs->tmtu=1400;   
	      memset (&hdr, 0, sizeof (struct headers));
	      rxs->peer_ip = tmp->peerip;
	      rxs->my_ip = tmp->myip;
	      rxs->pcx=tmp;
	      rxs->qnum=tmp->threads;
	      rxs->lb_delay.tv_sec=0;
	      rxs->lb_delay.tv_nsec=LBDELAY;
	      
             rxs->q = malloc (sizeof (Q) * tmp->threads);
                
                 fill_headers (rxs->pcx, &hdr, rxs->af);

	            for (j = 0; j < tmp->threads; j++) {
		     //  rxs->q[j]=malloc(sizeof(Q));
		    //////    rxs->q[j]->R = calloc (BUFSIZE/(rxs->mtu+sizeof(uint32_t)),(rxs->mtu+sizeof(uint32_t)));

		      init_ring_buffer (&rxs->q[j],BUFSIZE/tmp->threads, rxs->mtu, &hdr);
                      rxs->q[j].id=j;  
		new_thread (LBR, &rx, (void *)  rxs);
		
	      }
	      	      new_thread (IO, &rx_io, (void *) rxs);
	      	    //  new_thread (IO, &rx_lb, (void *) rxs);

	    }
	    else {
	      printf ("TUNTAP %s for %s not ready", tmp->tifname, tmp->name);
	      die (0, "TUNTAP error %s for %s not ready", tmp->tifname, tmp->name);
	    }
	  }
	  else if (strlen (tmp->socket_path) > 0) {
	    //don't judge me :P, I'll get to unix sockets later !! //TODO

	    /* rxs->type=UNIXSOCKET;
	       s = socket(AF_UNIX, SOCK_STREAM, 0);
	       if (s == -1) {
	       die(1,"Error creating unix socket");

	       }
	       uxs.sun_family= AF_UNIX;
	       strncpy(uxs.sun_path,tmp->socket_path,255);
	       unlink(uxs.sun_path);
	       sz=strlen(uxs.sun_path)+sizeof(uxs.sun_family);  
	       if (connect(s, (struct sockaddr *)&uxs, sz) == -1) {
	       die(1,"Error binding unix socket %s",uxs.sun_path);
	       }else{
	       printf("Unix socket %s is ready\r\n",tmp->socket_path);
	       rxs->fd=s; 
	       } */
	    //   init_ring_buffer((Q*)txtmp.q,BUFSIZE,rxs->mtu);
	  }
	  else if (strlen (tmp->pipe_path) > 0) {
	    int tmpfd = open (tmp->pipe_path, O_RDONLY | O_CREAT);

	    if (tmpfd < 0) {
	      die (1, "ERROR - %s - Unable to open named pipe %s for data source -", tmp->name,
		   tmp->pipe_path);
	      die (1,
		   "ERROR - No valid local data source specified for an Egress(out) tunnel %s\r\n",
		   tmp->name);
	    }
	    else {
	      rxs->type = PIPE;
	      rxs->fd = tmpfd;
	      //    init_ring_buffer ((Q *) & rxs->q, BUFSIZE, rxs->mtu);
	    }

	  }
	  else {
	    die (1,
		 "ERROR - No valid local data source specified for an Ingress(in) tunnel for %s \r\n",
		 tmp->name);
	  }

	}
	else {
	  die (1, "ERROR -%s- Unable to open raw socket for %s", tmp->name, tmp->ifname);
	}

      }
      else {
	die (1, "ERROR - No valid Egress(out) interface specified for an Ingress tunnel for %s",
	     tmp->name);
      }
}
int
init_af_packet (char *ifname, struct sockaddr_ll *sll) {
  int index, fd = -1;
  struct ifreq ifr;

  get_interface (ifname, &ifr, IFINDEX);
  if (ifr.ifr_ifindex < 1) {
    die (1, "ERROR - Unable to get interfce index for %s\r\n", ifname);
  }
  else {
    fd = socket (AF_PACKET, SOCK_RAW, htons (ETH_P_ALL));
    if(fd<1)die(1,"ERROR creating AF_PACKET socket for %s\r\n",ifname);
    
    bind(fd,(struct sockaddr *) sll,sizeof(struct sockaddr_ll));
    
  }

  return fd;
}

int
tuntap (char *dev, int flags) {

  struct ifreq ifr;
  int fd, err;
  char *devtun = "/dev/net/tun";

  if ((fd = open (devtun, O_RDWR)) < 0) {
    die (0, "Error setting up tunnel");
    return -1;
  }
  else {

    memset (&ifr, 0, sizeof (ifr));
    ifr.ifr_flags = flags;
    if (!(*dev) || strlen (dev) < 3 || strlen (dev) > IFNAMSIZ) {
      die (0, "Tunnel interface name error");
      return -1;
    }
    else {
      strncpy (ifr.ifr_name, dev, IFNAMSIZ);
      if (ioctl (fd, TUNSETIFF, (void *) &ifr) == -1) {
	die (0, "Error creating tunnel");
	return -1;

      }
      else {
	//do stuff once our tuntap is online..maybe? //TODO

      }

    }


  }


  return fd;
}

int
arp_request (int ifindex, char *src_mac, char *src_ip, char *dst_ip, uint8_t * result, int fd) {
  int status, i = 0, j, waited;
  uint8_t *ether_frame, asking[6];
  time_t clk;
  unsigned char ether_broadcast_addr[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };


  struct addrinfo hints, *res;
  struct sockaddr_in *ipv4;
  struct sockaddr_ll addr;
  arp_hdr *arphdr, *arpreply;

  ether_frame = (uint8_t *) malloc (IP_MAXPACKET * sizeof (uint8_t)), asking[4];

///request


  arphdr = malloc (sizeof (arp_hdr));

  memset (ether_broadcast_addr, 0xff, 6 * sizeof (uint8_t));
  memset (&hints, 0, sizeof (struct addrinfo));
  //setup the sockaddr to be used in sendto()
  memcpy (addr.sll_addr, src_mac, 6 * sizeof (uint8_t));
  addr.sll_family = AF_PACKET;
  addr.sll_ifindex = ifindex;
  addr.sll_halen = 6;
  addr.sll_protocol = htons (ETH_P_ALL);
  //fill up ethernet frame
  memcpy (ether_frame, ether_broadcast_addr, 6 * sizeof (uint8_t));
  memcpy (ether_frame + 6, src_mac, 6 * sizeof (uint8_t));
  ether_frame[12] = ETH_P_ARP / 256;
  ether_frame[13] = ETH_P_ARP % 256;
  memcpy (ether_frame + ETH_HDRLEN, &arphdr, ARP_HDRLEN * sizeof (uint8_t));

///resolve the target IP printable string to bits/bytes for the arp query
///getaddrinfo() allows for the dst_ip to be a hostname/domain name  
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = hints.ai_flags | AI_CANONNAME;
  if ((status = getaddrinfo (dst_ip, NULL, &hints, &res)) != 0) {
    fprintf (stderr, "getaddrinfo() failed: %s\n", gai_strerror (status));
    exit (EXIT_FAILURE);
  }
  ipv4 = (struct sockaddr_in *) res->ai_addr;
  memcpy (arphdr->target_ip, &ipv4->sin_addr, 4 * sizeof (uint8_t));
  freeaddrinfo (res);

  //inent_pton needs src_ip to be in printable form,converts it to network bytes
  //this IP will be the querying interface's IP
  if ((status = inet_pton (AF_INET, src_ip, arphdr->sender_ip)) != 1) {
    fprintf (stderr, "inet_pton() failed for source IP address.\nError message: %s",
	     strerror (status));
    exit (EXIT_FAILURE);
  }


//fill in the arp header
  arphdr->htype = htons (1);
  arphdr->ptype = htons (ETH_P_IP);
  arphdr->hlen = 6;
  arphdr->plen = 4;
  arphdr->opcode = htons (ARPOP_REQUEST);
  memcpy (arphdr->sender_mac, src_mac, 6 * sizeof (uint8_t));
  memset (arphdr->target_mac, 0, 6 * sizeof (uint8_t));
//copy the arp header onto the ethernet frame
  memcpy (ether_frame + ETH_HDRLEN, arphdr, ARP_HDRLEN * sizeof (uint8_t));
  free (arphdr);
//send the request
  if (sendto
      (fd, ether_frame, (6 + 6 + 2 + ARP_HDRLEN), 0, (struct sockaddr *) &addr,
       sizeof (addr)) == -1) {
    die (0, "%s", strerror (errno));
  }
  else {



// Get response:
    arpreply = (arp_hdr *) & ether_frame[ETH_HDRLEN];
    memcpy (asking, arpreply->target_ip, 4);	// "asking" is the IP we requested resolution for
    clk = time (NULL);		//time before we started waiting for an arp resolution

    while (memcmp (arpreply->sender_ip, asking, 4) != 0)	//when arp resolves sender_ip should match the IP we queried 
    {
      if ((status = recv (fd, ether_frame, IP_MAXPACKET, 0)) < 0) {
	if (errno == EINTR) {
	  memset (ether_frame, 0, IP_MAXPACKET * sizeof (uint8_t));
	  continue;		// Something weird happened, but let's try again.
	}
	else {
	  die (0, "ARP recv() failed:");
	  return -1;
	}

      }

      //note the arp time out is not set in stone, the loop will hang until recv() above returns something
      waited = ((int) time (NULL)) - ((int) clk);
      if (waited > ARP_TIMEOUT) {
	die (0, "ARP time-out for %s\r\n", dst_ip);
	return -1;
      }
    }				//loop is done,means arp is done resolving :)



    memcpy (result, arpreply->sender_mac, 6);
  }
  return 0;

}

uint32_t
host_to_ip (char *host) {

  struct addrinfo hints, *hinfo;
  struct sockaddr_in *res;
  uint32_t *ip;

  memset (&hints, 0, sizeof (hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_flags = AI_PASSIVE;

  if (getaddrinfo (host, NULL, &hints, &hinfo))
    return 0;

  res = (struct sockaddr_in *) hinfo->ai_addr;
  ip = (uint32_t *) & res->sin_addr;
  return *ip;


}

int
set_ip (char *if_name, uint32_t ip) {
  struct ifreq ifr;
  struct sockaddr_in *skin;

  size_t if_name_len = strlen (if_name);

  if (if_name_len < sizeof (ifr.ifr_name)) {
    memcpy (ifr.ifr_name, if_name, if_name_len);
    ifr.ifr_name[if_name_len] = 0;
  }
  else {
    die (1, "interface name is too long");
  }
  int fd = socket (AF_INET, SOCK_DGRAM, 0);

  if (fd == -1) {
    die (1, "Unable to set IP on interface %s", if_name);
  }
  else {
    ifr.ifr_addr.sa_family = AF_INET;
    skin = (struct sockaddr_in *) &ifr.ifr_addr;
    memcpy (&skin->sin_addr, &ip, sizeof (uint32_t));
    if (ioctl (fd, SIOCSIFADDR, &ifr) == -1) {
      die (1, "IOCTL to set IP failed %s", if_name);
    }

  }
}

void
set_mtu (char *if_name, int mtu) {
  struct ifreq ifr;

  size_t if_name_len = strlen (if_name);

  if (if_name_len < sizeof (ifr.ifr_name)) {
    memcpy (ifr.ifr_name, if_name, if_name_len);
    ifr.ifr_name[if_name_len] = 0;
  }
  else {
    die (1, "interface name is too long");
  }
  int fd = socket (AF_INET, SOCK_DGRAM, 0);

  if (fd == -1) {
    die (1, "Unable to set MTU %i on interface %s", mtu, if_name);
  }
  else {
    ifr.ifr_mtu = mtu;

    if (ioctl (fd, SIOCSIFMTU, &ifr) == -1) {
      die (1, "IOCTL to set IP failed %s mtu %i", if_name, mtu);
    }

  }
}

void
fill_headers (struct peer_context *pcx, struct headers *hdr, int fd) {
  struct ifreq ifr;
  uint8_t *smac;
  int index;
  struct sockaddr_in *ipaddr;

  memset (hdr, 0, sizeof (struct headers));

  /**** Ethernet ****/
  get_interface (pcx->ifname, &ifr, IFMAC);
  smac = (uint8_t *) ifr.ifr_hwaddr.sa_data;
  memcpy (&hdr->eh.h_source, smac, 6);
  get_interface (pcx->ifname, &ifr, IFINDEX);
  index = ifr.ifr_ifindex;
  get_interface (pcx->ifname, &ifr, IFADDR);
  ipaddr = (struct sockaddr_in *) &ifr.ifr_addr;

  arp_request (index, &hdr->eh.h_source[0], inet_ntoa (ipaddr->sin_addr), pcx->next_hop,
	       hdr->eh.h_dest, fd);

  hdr->eh.h_proto = htons (ETH_P_IP);

  /***** ipv4 *****/

  hdr->iph.ihl = 5, hdr->iph.version = 4;
  hdr->iph.tos = htons (16);
  hdr->iph.tot_len = 1500;	//this should probably be filled during runtime.
  hdr->iph.id = 0;
  hdr->iph.frag_off = 0;
  hdr->iph.ttl = 255;
  hdr->iph.protocol = 1;
  hdr->iph.check = 0;		//runtime
  hdr->iph.saddr = (pcx->myip);
  hdr->iph.daddr = (pcx->peerip);

}
/*** Copied straight out of the kernel docs :P ***/
  int tun_set_queue(int fd, int enable)
  {
      struct ifreq ifr;

      memset(&ifr, 0, sizeof(ifr));

      if (enable)
         ifr.ifr_flags = IFF_ATTACH_QUEUE;
      else
         ifr.ifr_flags = IFF_DETACH_QUEUE;

      return ioctl(fd, TUNSETQUEUE, (void *)&ifr);
  }
void set_fanout(int fanout_id,int af){
int fanout_arg = (fanout_id | PACKET_FANOUT_CPU);

	if(setsockopt(af, SOL_PACKET, PACKET_FANOUT,
			 &fanout_arg, sizeof(fanout_arg))){
	  die(1,"Error setting AF_PACKET Fanout\n");
	}
	
 
}
void
trace_dump (char *msg, RNG * data) {
  int i = 0;

  printf
    ("\r\n+----------------------------%i{%s}len:%i-----------------------------------------+\r\n",
     data->index, msg, data->len);

  for (i = 0; i < ETH_HDRLEN; i++) {
    printf ("\033[1;32m%02x", data->ethernet_frame[i]);
  }
  printf ("\r\n\0******************************************************************************\r\n");
  for (i; i < ETHIP4; i++) {
    printf ("\033[1;33m%02x", data->ethernet_frame[i]);
  }
  printf ("\r\n******************************************************************************\r\n");
  for (i; i < data->len + (ETHIP4); i++) {
    printf ("\033[1;34m%02x", data->ethernet_frame[i]);
  }
  printf("\n");
}

void free_tx(struct txq *h){
  free(h->q->R);
  free(h->q);
  free(h);
  
}

void free_rx(struct rxq *h){
    free(h->q->R);
  free(h->q);
  free(h);
}
void free_map(struct mapq *h){
    free(h->q->R);
  free(h->q);
  free(h);
}