#include "threads.h"
#include <stdlib.h>

#define DEBUG

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t underline = PTHREAD_COND_INITIALIZER;
pthread_cond_t big = PTHREAD_COND_INITIALIZER;
pthread_cond_t small = PTHREAD_COND_INITIALIZER;
//pthread_cond_t match = PTHREAD_COND_INITIALIZER;

int fish_sum;
char last;
// Allowed libraries:
// * pthread_mutex_lock, pthread_mutex_unlock
// * pthread_cond_wait, pthread_cond_signal, pthread_cond_broadcast
// * sem_init, sem_wait, sem_post
void fish_init() {
	last = 0;
	fish_sum = 0;
	srand((int)time(NULL));
  // TODO
}

void fish_before(char ch) {
	pthread_mutex_lock(&lk);
	while(!cond) pthread_cond_wait(&cond, &lk);

	/*
	pthread_mutex_lock(&lock);
	if(ch == '_') {
	    while ( fish_sum < 180 ) {
		    pthread_cond_wait(&underline, &lock);
		}
	}
    
	else if(ch == '<') {
	    while( last != '>' && fish_sum != 0) {
		    pthread_cond_wait(&small, &lock);
	//	    pthread_cond_wait(&match, &lock);
		}
	}

	else {
	    while( last != '<' && fish_sum != 0) {
		    pthread_cond_wait(&big, &lock);
	//	    pthread_cond_wait(&match, &lock);
		}
	}

    pthread_mutex_unlock(&lock);
  // TODO
  */
}

void fish_after(char ch) {
	pthread_mutex_lock(&lock);
	fish_sum += ch;
	last = ch;

	if(fish_sum > 240) {
		fish_sum = 0;
	}

	if(fish_sum > 180) {
	    pthread_cond_signal(&underline);
	}
    
	else {
	    if(ch == '>') {
	        pthread_cond_signal(&small);
		}
		else if(ch == '<'){
	        pthread_cond_signal(&big);
		}
		else {
	        int i = rand()%2;
	        if(i == 0) {
	            pthread_cond_signal(&small);
		    }	
		    else {
	            pthread_cond_signal(&big);
		    }
		}
	}

    pthread_mutex_unlock(&lock);
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

