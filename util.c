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
  static int i=0;
  pthread_t *handle = malloc (sizeof (pthread_t));
  struct thread_management *tm = malloc (sizeof (struct thread_management));
  
  tm->T = handle;
  tm->job = job;
  i=GTS.gtc;
  tm->id =i;
  ++GTS.gtc;
  
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
  char msg[245],color[256]="\033[1;31m";
  va_list arglist;

  va_start (arglist, why);
  
  vsnprintf (msg, 245, why, arglist);
  strncat(color,msg,256);
  va_end (arglist);

  if (really) {

    perror (color);
    debug (4, color);
    exit (really);
  }
  else {
    debug (4, color);
    perror (color);
  }

}

unsigned int
g_rand () {
  srand (time (NULL));
  return rand () % 1000 + 1;
}

void
debug (int level, char *s,...) {

  if (dbglvl >= level) {
//    logg (s);
  va_list arglist;

  va_start (arglist, s);
  vprintf (s, arglist);
  va_end (arglist);
  //printf ("\n");
  fflush (stdout);
  
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

inline int atomic_islocked(volatile int L){
 int test=1; 
 __sync_fetch_and_and(&test,L);
 
 return test==1 ? test : 0;
  
}
inline int atomic_lock(volatile int *L){
 return __sync_val_compare_and_swap(L,0,1); 
}

inline int atomic_unlock(volatile int *L){
 return __sync_val_compare_and_swap(L,1,0); 
}
inline int atomic_cond_lock(volatile int *L){
 
 return __sync_val_compare_and_swap(L,0,1);
}

inline void adaptive_spin (struct timespec *ts,volatile int watch,volatile int *spins){
  const int inc=1000;
  
  if (ts->tv_nsec < inc){
    ts->tv_sec=0;
    ts->tv_nsec=inc;
    *spins=0;
  }
  else if (*spins < 1000000 && ts->tv_nsec >=250000){
   ts->tv_nsec= ts->tv_nsec/2;
   *spins -= 100000;
  }else if(ts->tv_nsec >= 500000){
   ts->tv_nsec= inc; 
  }
  ts->tv_nsec += inc;
  
  while(watch==0){
   ++(*spins);
   nanosleep(ts,NULL);
   debug(6,"\r nsec %i , spins %i",ts->tv_nsec,*spins);
  }
  printf("\n");
    
  
}