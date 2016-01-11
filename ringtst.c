//#include "ring.h"
/////ring.h
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>


int bsz, rsz, total = 0, lost = 0, ploss;

typedef struct queue {
  pthread_mutex_t mutex;

  int in;
  int out;
  int sz;
  unsigned char *rng;

} Q;

typedef struct tststruct {
  unsigned int id;
  char s[1500];
  Q *T;
} test;

int enqueue (char *s, Q * q);
char *pop (Q * q);
void init (Q * q, int size, int chunks);

//////////////////////////////////////////////////////
///ring.c
unsigned int
g_rand () {
  srand (time (NULL));
  return rand () % 100 + 1;
}

void
init (Q * q, int size, int chunks) {
  memset (q, 0, sizeof (Q));
  bsz = chunks;
  rsz = (size / chunks);
  q->rng = calloc (rsz + bsz, bsz);
}

int
enqueue (char *s, Q * q) {

  if (q->in >= (rsz - bsz)) {
    q->in = 0;
  }
  if (q->sz >= rsz) {
    ++lost;
    return 1;
  }

//char *new=(char *)(&q->rng+(bsz*q->in++));

  memcpy (&q->rng[q->in], s, bsz);
  // q->rng[bsz]='\0';
  q->in += bsz;

  q->sz += bsz;
  total += bsz;

  return 0;
}


char *
pop (Q * q) {
  if (q->out >= (rsz - bsz)) {
    q->out = 0;
  }
  if (q->sz < bsz) {
    return NULL;
  }
  q->sz -= bsz;
//char *res=(char *) q->rng+(bsz*q->out++);
  char *p = &q->rng[q->out];

  q->out += bsz;
  return p;
}



void *
input (void *arg) {
  test *t = (test *) arg;
  int res;

  printf ("Starting %i\r\n", t->id);
  while (1) {
    if (pthread_mutex_lock (&t->T->mutex) == 0) {
      res = enqueue (t->s, t->T);
      pthread_mutex_unlock (&t->T->mutex);

      if (res) {
	//   printf("!"); //dropped one
	fflush (stdout);
      }
      else {
	//     printf("\r\n[%i]Pushing >>%s<< \r\n",t->id,t->s);
	//fflush(stdout);
	res = 0;
      }
//sleep(2);
    }

  }
}


void *
output (void *arg) {
  test *t = (test *) arg;

  printf ("Starting %i\r\n", t->id);
  char *o;

  while (1) {
    //sleep(2);
    if (pthread_mutex_lock (&t->T->mutex) == 0) {

      o = pop (t->T);
      pthread_mutex_unlock (&t->T->mutex);

      if (o != NULL) {
	printf ("\r Total:%i      Lost:%i     Loss-percent:%g", total, lost,
		(((double) lost) / total) * 100);
	fflush (stdout);
      }
      else {

	fflush (stdout);
      }
    }
  }


}

int
main () {
  char s[1500];
  int c = 64, i = 0, j = 0;
  pthread_t ti, to;

  Q T;

  init (&T, 70000000, 1500);


  for (i = 0; i < 10; i++) {
    for (j = 0; j < bsz; j++)
      s[j] = c;

    ++c;
    s[bsz - 1] = '\0';
    test *t = malloc (sizeof (test));

    memset (t, 0, sizeof (test));
    t->id = i;
    memcpy (t->s, s, bsz);
    t->T = &T;

    ti = (pthread_t) g_rand ();
    pthread_create (&ti, 0, &input, (void *) t);

  }



  for (i = 0; i < 10; i++) {
    test *t = malloc (sizeof (test));

    memset (t, 0, sizeof (test));
    t->id = 900 + i;
    t->T = &T;
    to = (pthread_t) g_rand ();
    pthread_create (&to, 0, &output, (void *) t);

  }

  pthread_join (to, NULL);





  return 0;
}
