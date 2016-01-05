#include "util.h"

void
init_peer_list () {

  INIT_LIST_HEAD (&MPL.PL);


}

void
init_thread_list () {

  INIT_LIST_HEAD (&MTL.TL);


}

void
add_to_peer_list (struct peer_context *pc) {

  list_add (&(pc->PL), &(MPL.PL));
}

void
add_to_thread_list (struct thread_management *t) {

  list_add (&(t->TL), &(MTL.TL));
}

void
peer_list_dump () {

  struct list_head *lh;
  struct peer_context *tmp;


  list_for_each (lh, &(MPL.PL)) {

    tmp = list_entry (lh, struct peer_context, PL);

    if (tmp != NULL)
      printf
	(">>> Id: %i, Direction: %i, Name: %s , Interface %s , Peer-ip: %s, Cipher: %s, Key %s\r\n",
	 tmp->id, tmp->direction, tmp->name, tmp->ifname, tmp->peer_ip, tmp->cipher, tmp->key);

  }
}

struct thread_management *
new_thread (int job, void *(*start_routine) (void *), void *arg) {

  pthread_t *handle = malloc (sizeof (pthread_t));
  struct thread_management *tm = malloc (sizeof (struct thread_management));

  tm->T = handle;
  tm->job = job;
  tm->id = ++GTS.gtc;
  tm->id;
  tm->arg=arg;
  pthread_create (handle, 0, start_routine, arg);
  switch (job) {
	  case SUP:
	    GTS.supc++;
	    pthread_join (*handle, NULL);
	    break;
	  case IO:
	    // pthread_detach(*handle);
	    GTS.ioc++;
	    break;
	  case LBR:
	    //pthread_detach(*handle);
	    GTS.lbrc++;
	    break;
	  default:
	    exit (1);		//should never get here
  }
  
  
  add_to_thread_list (tm);	//add self to Master thread list

  return tm;
}

void
die (int really, char *why, ...) {
  char msg[256];
  va_list arglist;

  va_start (arglist, why);
  snprintf(msg,10,"\033[1;31m");
  vsnprintf (&msg[11], 255, why, arglist);
  va_end (arglist);

  if (really) {

    perror (msg);
    debug (4, msg);
    exit (really);
  }
  else {
    debug (4, msg);
    perror (msg);
  }

}

unsigned int
g_rand () {
  srand (time (NULL));
  return rand () % 1000 + 1;
}

void
debug (int level, char *s) {

  if (dbglvl >= level) {
    logg (s);

  }
}

void
logg (char *s, ...) {

  va_list arglist;

  va_start (arglist, s);
  vprintf (s, arglist);
  va_end (arglist);
  printf ("\n");
  fflush (stdout);
}

unsigned short
csum (unsigned short *buf, int nwords) {
  unsigned long sum;

  for (sum = 0; nwords > 0; nwords--)
    sum += *buf++;
  sum = (sum >> 16) + (sum & 0xffff);
  sum += (sum >> 16);
  return (unsigned short) (~sum);
}
//soon.... //TODO
void drop_privs(){
  
  
}

void signal_handler(int sig){ //TODO
  
  
}