#ifndef NETWORK_H
#define NETWORK_H

#include <errno.h>
#include <netinet/in.h>
#include <linux/if_tun.h>
#include <netinet/if_ether.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/ether.h>
#include <netinet/ip.h>
#include <net/if.h>
#include<stdarg.h>
#include<stdio.h>
#include <netdb.h>
#include <time.h>



#include "main.h"
#include "util.h"
#include "list.h"
#include "workers.h"
#include "ring.h"




//In bytes  this should amunt to 60MB, ideal 50ms of "line rate output buffer" on 10G
//buffer bloat is possible at this time,further testing and ethtool integration
//needs to be done.
#define BUFSIZE 62914560

void get_interface (char *if_name, struct ifreq *ifr, int req);
int set_ip (char *ifname, uint32_t ip);
void set_mtu (char *ifname, int mtu);
int enumerate_interfaces ();
int ifup (char *if_name);
int tuntap (char *dev, int flags);
int init_af_packet (char *ifname, struct sockaddr_ll *sll);
uint32_t host_to_ip (char *host);
void fill_headers (struct peer_context *pcx, struct headers *hdr, int fd);
void trace_dump (char *msg, RNG * r);

/*** arp_request() returns 0 on success, -1 on failure.
 * ifindex - interface index from ioctl call using get_interface()
 * src_mac - also using get_interface() the mac address of the sending interface
 * src_ip - IP address of the source interface also aquired usign get_interface()
 * dst_ip - the IP we want to resolve - this should be the gateway IP configured for each peer
 * result - this is where the resolved mac is stored
 * fd - raw socket file descripter - should already be open
 * ***************************************/
int arp_request (int ifindex, char *mac, char *src_ip, char *dst_ip, uint8_t * result, int fd);




struct txq {
  char type;
  char ifname[IFNAMSIZ];
  char tifname[IFNAMSIZ];

  int fd;
  int af;
  int mtu;
  int tmtu;
  int lost_in;
  int lost_out;
  int packets_in;
  int packets_out;
  int bytes_in;
  int bytes_out;

  uint32_t peer_ip;
  uint32_t my_ip;

  struct sockaddr_ll sll;
  struct thread_management *tmh;
  Q *q;
};

struct mapq {
  char type;
  char ifname[IFNAMSIZ];
  char mapifname[IFNAMSIZ];

  int map;
  int mtu;
  int af;
  int afmap;

  int lost_in;
  int lost_out;
  int packets_in;
  int packets_out;
  int bytes_in;
  int bytes_out;

  struct ether_header eh;
  struct sockaddr_ll sll;
  struct sockaddr_ll sllmap;
  struct thread_management *tmh;
  Q *q;
};
struct rxq {
  char type;
  char ifname[IFNAMSIZ];
  char tifname[IFNAMSIZ];

  int fd;
  int af;
  int mtu;
  int tmtu;

  int lost_in;
  int lost_out;
  int packets_in;
  int packets_out;
  int bytes_in;
  int bytes_out;

  uint32_t peer_ip;
  uint32_t my_ip;

  struct ether_header eh;
  struct sockaddr_ll sll;
  struct thread_management *tmh;
  Q *q;
};

struct txq *TX;
struct rxq *RX;
struct mapq *M;

///////////////////////////////////////////
typedef struct _arp_hdr arp_hdr;
struct _arp_hdr {
  uint16_t htype;
  uint16_t ptype;
  uint8_t hlen;
  uint8_t plen;
  uint16_t opcode;
  uint8_t sender_mac[6];
  uint8_t sender_ip[4];
  uint8_t target_mac[6];
  uint8_t target_ip[4];
};

/** does what it says **/
void print_tx_stats (struct txq* q);
void print_rx_stats (struct rxq* q);
void print_map_stats (struct mapq* q);
#endif
