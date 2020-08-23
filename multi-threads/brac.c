#include "threads.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX 4
#define LENGTH(arr) (sizeof(arr)/sizeof(arr[0])) 

//int empty = MAX;
//int fill = 0;

char buff[5] = {'\0', '\0', '\0', '\0', '\0'};

struct state {
    char buff[5];
};

typedef struct state state;

state states[] = {
    { "((((\0" },
	{ "[(((\0" },
	{ "[[((\0" },
	{ "[[[(\0" },
	{ "[[[[\0" },
	{ "(((\0\0" },
	{ "[((\0\0" },
	{ "[[(\0\0" },
	{ "[[[\0\0" },
	{ "((\0\0\0" },
	{ "[(\0\0\0" },
	{ "[[\0\0\0" },
	{ "(\0\0\0\0" },
	{ "[\0\0\0\0" },
};

int test_producer(char ch) {
    for(int i = 0; i < LENGTH(states); i ++) {
		char* tmp = (char*)malloc(5*sizeof(char));
		char* bit = (char*)malloc(2*sizeof(char));
		memset(tmp, '\0', 5);
		memset(bit, '\0', 2);
		bit[0] = ch;
		strcpy(tmp, buff);
		strcat(tmp, bit);
	    if(strcmp(tmp, states[i].buff) == 0) {
		    return 1;
		}
        free(tmp);
		free(bit);
	}

	return 0;
}

int test_consumer(char ch) {
	int len = strlen(buff);
	if(len < 1) return 0;

	int bit_num = 0;
    for(bit_num; bit_num < 4; bit_num ++) {
	    if(buff[bit_num] == '\0') {
		    break;
		}
	}

	bit_num --;
	if(buff[bit_num] == '(' && ch == ')') return 1;
	if(buff[bit_num] == '[' && ch == ']') return 1;

	return 0;
}

int count = 0;

int quota = 1;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
/*
pthread_cond_t cvse = PTHREAD_COND_INITIALIZER;
pthread_cond_t cvbe = PTHREAD_COND_INITIALIZER;
pthread_cond_t cvsf = PTHREAD_COND_INITIALIZER;
pthread_cond_t cvbf = PTHREAD_COND_INITIALIZER;
*/

pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t fill = PTHREAD_COND_INITIALIZER;

// Allowed libraries:
// * pthread_mutex_lock, pthread_mutex_unlock
// * pthread_cond_wait, pthread_cond_signal, pthread_cond_broadcast
// * sem_init, sem_wait, sem_post
void brac_init() {
  // TODO
}

void brac_before(char ch) {
	pthread_mutex_lock(&mutex);

    if(ch == '[' || ch == '(') {
	    while(test_producer(ch) == 0 || quota != 1 || count == 4) COND_WAIT(&empty, &mutex);

		quota --;
	}

	if(ch == ')' || ch == ']') {
	    while(test_consumer(ch) == 0 || quota != 1 || count == 0) COND_WAIT(&fill, &mutex);

		quota --;
	}  

	pthread_mutex_unlock(&mutex);

  // TODO
}

void brac_after(char ch) {
	pthread_mutex_lock(&mutex);

    if(ch == '[' || ch == '(') {
		char* bit = (char*)malloc(2*sizeof(char));
		memset(bit, '\0', 2);
		bit[0] = ch;
		strcat(buff, bit);
		count ++;
		quota ++;
		free(bit);
	    
		COND_BROADCAST(&fill);

	}

	if(ch == ')' || ch == ']') {
		int len = strlen(buff);
		memset(&buff[len-1], '\0', 1);
		count --;
		quota ++;
	    
		COND_BROADCAST(&empty);
	}  

	pthread_mutex_unlock(&mutex);
  // TODO
}

static const char roles[] = "[[]](())";

void brac_thread(int id) {
  char role = roles[id];
  while (1) {
  //for(int i = 0; i < 12; i ++) {
    brac_before(role);
    putchar(role); // should not hold *any* mutex lock now
	fflush(stdout);
    brac_after(role);
  }
}

void SayBye() {
    printf("Done!\n");
}

int main() {
  setbuf(stdout, NULL);
  brac_init();
  for (int i = 0; i < strlen(roles); i++)
    create(brac_thread);
  join(SayBye);
}

