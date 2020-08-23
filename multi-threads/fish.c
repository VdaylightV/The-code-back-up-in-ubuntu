#include "threads.h"
#include <stdlib.h>
#include <string.h>

#define DEBUG
#define LENGTH(arr) (sizeof(arr)/sizeof(arr[0]))

enum state {
   A = 1, B, C, D, E, F,
};

struct rule {
    enum state from;
	char ch;
	enum state to;
};

struct rule rules[] = {
 {A, '>', B},
 {B, '<', C},
 {C, '>', D},
 {D, '_', A},
 {A, '<', E},
 {E, '>', F},
 {F, '<', D},
};

enum state current = A;

int quota = 1;

int next(char ch) {
    for(int i = 0; i < LENGTH(rules); i ++) {
		if(rules[i].from == current && rules[i].ch == ch) {
		    return rules[i].to;
		}
	}
	return 0;
}

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;

// Allowed libraries:
// * pthread_mutex_lock, pthread_mutex_unlock
// * pthread_cond_wait, pthread_cond_signal, pthread_cond_broadcast
// * sem_init, sem_wait, sem_post
void fish_init() {
  // TODO
}

void fish_before(char ch) {
	pthread_mutex_lock(&mutex);

	while(next(ch) == 0 || quota == 0) COND_WAIT(&cv, &mutex);
    
	assert(next(ch) != 0 && quota == 1);

	quota --;
    
//	assert(cond);

	pthread_mutex_unlock(&mutex);

  // TODO
}

void fish_after(char ch) {
	pthread_mutex_lock(&mutex);

	current = next(ch);

	quota ++;

	COND_BROADCAST(&cv);
	pthread_mutex_unlock(&mutex);
  // TODO
}

static const char roles[] = "<<<<<>>>>___";

void fish_thread(int id) {
  char role = roles[id];
  while (1) {
    fish_before(role);
    putchar(role); // should not hold *any* mutex lock now
    fish_after(role);
  }
}

int main() {
  setbuf(stdout, NULL);
  fish_init();
  for (int i = 0; i < strlen(roles); i++)
    create(fish_thread);
  join(NULL);
}

