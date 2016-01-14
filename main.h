#ifndef MAIN_H
#define MAIN_H

#include "util.h"
#include "list.h"

#include <net/if.h>
#include <stdint.h>


#define IN 0
#define OUT 1
#define MAP 2

struct global_settings {
  char instance_name[64];
  unsigned int fips_compliant;	//meh..whatever maybe someday :P //TODO
  char config_file[256];
  
  int debug;
};
struct peer_context {
  int id;
  int ip_version;
  int direction;
  int threads;

  uint32_t peerip;
  uint32_t myip;


  char name[256];
  char version;
  char ifname[IFNAMSIZ];
  char tifname[IFNAMSIZ];
  char ifmap[IFNAMSIZ];
  char peer_ip[256];
  char next_hop[256];
  char cipher[256];
  char socket_path[256];
  char pipe_path[256];
  char key[16000];
  struct list_head PL;
};

struct global_settings global;

#endif
