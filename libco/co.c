#include "co.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <setjmp.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define KB (1<<10)
#define STACK_SIZE 64*KB
#define POOL_SIZE 128

#define POS
#define CO_START_CHK

static inline void stack_switch_call(void *sp, void *entry, uintptr_t arg) {
  asm volatile (
#if __x86_64__
    "movq %0, %%rsp; movq %2, %%rdi; jmp *%1"
      : : "b"((uintptr_t)sp), "d"(entry), "a"(arg)
#else
    "movl %0, %%esp; movl %2, 4(%0); jmp *%1"
      : : "b"((uintptr_t)sp - 8), "d"(entry), "a"(arg)
#endif
  );
}

enum co_status {

    CO_NEW = 1,
	CO_RUNNING,
	CO_WAITING,
	CO_DEAD,

};

struct co {
    bool tag;

	char *name;
	void (*func)(void *);
	//void (*func)(void *);
	void *arg;

	enum co_status status;
    struct co*     waiter;
	jmp_buf        context;
	uint8_t stack[STACK_SIZE];

};

int pool_member = 0;
struct co* pool[128];
struct co current_co = {true, NULL, NULL, NULL, CO_RUNNING, NULL};
struct co* current = &current_co;
char main_name[5] = "main";

void co_delete(struct co* co) {
#ifdef POS
	printf("#[POS]:In co_delete\n");
#endif
    for(int i = 0; i < pool_member; i ++) {
        if(!strcmp(pool[i]->name, co->name)) {
            pool[i] = NULL;
            break;
        }
        if(pool[i] == NULL) {
            continue;
        }
    }
}

struct co* rand_choose() {
#ifdef POS
	printf("#[POS]:In rand_choose\n");
#endif
    srand((unsigned int)time(0));
    struct co* co = NULL;
    int index = 0;
    do {
        index = rand() % pool_member;
        if(pool[index] != NULL && (pool[index]->status == CO_NEW || pool[index]->status == CO_WAITING)) {
            co = pool[index];
            break;
        }
        else {
            if(index >= 1 && (pool[index-1] != NULL && (pool[index-1]->status == CO_NEW || pool[index-1]->status == CO_WAITING))) {
                co = pool[index-1];
                break;
            }

            else if(index <= pool_member-2 && (pool[index+1] != NULL && (pool[index+1]->status == CO_NEW || pool[index+1]->status == CO_WAITING))) {
                co = pool[index+1];
                break;
            }
        }
    } while(co == NULL);

    return co;
}

void callback() {
#ifdef POS
	printf("#[POS]:In callback\n");
#endif
    current->status = CO_DEAD;
    co_delete(current);
    struct co* co = NULL;
    do {
        co = rand_choose();
    } while(!strcmp(co->name, current->name));
    current = co;
    longjmp(current->context, 2);

}

void set_ret_addr(struct co* co) {
#ifdef POS
	printf("#[POS]:In set_ret_addr\n");
#endif
#if __x86_64__
    uint64_t num = (uintptr_t)callback;
    for(int i = 0; i < 16; i ++) {
		co->stack[STACK_SIZE-28+i] = num % 256;	    
		num /= 256;
	}	
#else
    uint32_t num = (uintptr_t)callback;
    for(int i = 0; i < 4; i ++) {
		current->stack[STACK_SIZE-36+i] = num % 256;	    
		num /= 256;
	}	
#endif
}

struct co *co_start(const char *name, void (*func)(void *), void *arg) {
#ifdef POS
	printf("#[POS]:In co_start\n");
#endif
    assert(name != NULL && func != NULL && arg != NULL);

    struct co* new_co = (struct co*)malloc(sizeof(struct co));
    strcpy();
#ifdef CO_START_CHK
	printf("Everything is OK\n");
#endif
    new_co->func = func;
    new_co->tag = false;
    new_co->arg = arg;
    new_co->status = CO_NEW;
    new_co->waiter = NULL;
    for(int i = 0; i < STACK_SIZE; i ++) {
        new_co->stack[i] = 0;
    }

    pool[pool_member] = new_co;
    pool_member ++;

    return new_co;
}

void current_xchg(struct co *co) {
#ifdef POS
	printf("#[POS]:In current_xchg\n");
#endif

    struct co* old_current = current;
    old_current->status = CO_WAITING;
    current = co;
    current->waiter = old_current;
    current->status = CO_RUNNING;
    return;

}

void co_wait(struct co *co) {
#ifdef POS
	printf("#[POS]:In co_wait\n");
#endif
    assert(co != NULL);
    if(co->status != CO_DEAD && co->status != CO_WAITING) {
        current_xchg(co);
        current->func(current->arg);

        current->status = CO_DEAD;

        co_delete(current);

        current = current->waiter;
        free(co);

        assert(current->tag == true);

        return;
    }

    else if(co->status == CO_DEAD) {
        free(co);        
        return;
    }

    else if(co->status == CO_WAITING) {
        co_yield();
        return;
    }
}

void co_yield() {
#ifdef POS
	printf("#[POS]:In co_yield\n");
#endif
    current->status = CO_WAITING;
//    struct co* old_current = current;
    int val = setjmp(current->context);
    if (val == 0) {
        struct co* choice = rand_choose();
        if(choice->status == CO_NEW) {
            set_ret_addr(choice);    
            current = choice;
            current->status = CO_RUNNING;
            stack_switch_call(&choice->stack[STACK_SIZE-32], choice->func, (uintptr_t)choice->arg);
        }

        else if(choice->status == CO_WAITING) {
            choice->status = CO_RUNNING;
            current = choice;
            longjmp(choice->context, 2);
        }

        else {
            assert(0);
        }
    }
    else {
        return;
    }
}
