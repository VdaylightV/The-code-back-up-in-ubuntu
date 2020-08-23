#include "co.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <setjmp.h>
#include <string.h>
#include <time.h>

#define STACK_SIZE ((1<<16))

//#define DEBUG
//#define JMP
//#define TEST_2
//#define CO_DELETE
//#define BUG
//#define CURCHK
//#define STACK

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

	char *name;
//	__attribute__ ((aligned (16))) void (*func)(void *);
	void (*func)(void *);
	void *arg;

	enum co_status status;
	struct co *    next; // to connect members in list
	struct co *    brother; // to connect members in rand_pool
	jmp_buf        context;
	uint8_t        stack[STACK_SIZE];

}__attribute__ ((aligned (16)));

struct co *current = NULL;

struct co co_list;
struct co *co_list_head = &co_list;

struct co rand_pool;
struct co *rand_pool_head = &rand_pool;

void stack_display(uint8_t* addr, int num) {
#if __x86_64__
	for(int i = 0; i < num; ) {
		if(i % 8 == 0) {
		    printf("%p: ", &addr[i]);
		}
		printf("%04d ", addr[i]);
		if((i+1) % 8 == 0) {
		    printf("\n");
		}
		i ++;
	}	
	printf("\n");
#else
	for(int i = 0; i < num; ) {
		if(i % 4 == 0) {
		    printf("%p: ", &addr[i]);
		}
		printf("%04d ", addr[i]);
		if((i+1) % 4 == 0) {
		    printf("\n");
		}
		i ++;
	}	
	printf("\n");

#endif
}

void stack_head_chk() {
#if __x86_64__
    uint64_t head = 0;
	asm volatile (
		"movq %%rsp, %0;"
		: "=r"(head) 
		: 
		: 
	);
	printf("rsp: 0x%ld\n", head);
	printf("stack bottom: 0x%ld\n", (uint64_t)current->stack);
	//assert((uint64_t)current->stack < head);
#else
    uint32_t head = 0;
	asm volatile (
		"movl %%esp, %0;"
		: "=r"(head)
		:
	   	: 
	);
	printf("esp: 0x%d\n", head);
	printf("stack bottom: 0x%d\n", (uint32_t)current->stack);
	//assert((uint32_t)current->stack < head);
#endif
}

void list_append(struct co* head, struct co* new_co) {
    struct co* temp = head;
	while(temp->next != NULL) {
	   	temp = temp->next;
	}
	temp->next = new_co;
	return;
}

void current_chk() {
#ifdef CURCHK
	if(current == NULL) {
	    printf("***{CURRENT CO}: co main\tLINE:");
	}
	else {
	    printf("***{CURRENT CO}: co %s\tLINE", current->name);
	}
#endif
	return;
}

void co_list_check()
{
    assert(co_list_head->next == NULL);
}

void rand_pool_append(struct co* head, struct co* new_co) {
   	struct co* temp = head;
	while(temp->brother != NULL) {
    	temp = temp->brother;
	}
	temp->brother = new_co;
	return;
}

void co_count() {
    assert(co_list_head->next != NULL);
	int count = 0;
	struct co* temp = co_list_head->next;
	while(temp != NULL) {
	    count ++;
		temp = temp->next;
	}
	printf("\n$$$ There are %d co(s) in list\n\n", count);
	return;
}

void co_delete(struct co* node) {
#ifdef CO_DELETE
	printf("***In co_delete function\n");
#endif
    struct co* temp = co_list_head->next;
	struct co* before_temp = co_list_head;
	while(temp != NULL) {
	    if(!strcmp(temp->name, node->name)) {
			struct co* next = temp->next;
			if(next == NULL) {
			    before_temp->next = NULL;
				break;
			}	
			else {
				assert(next != NULL);
				before_temp->next = next;
				break;
			}
		}
		else {
		    before_temp = temp;
			temp = temp->next;
		}
	}
#ifdef CO_DELETE
	printf("co %s was deleted from co_list\n", node->name);
#endif
#ifdef BUG
	printf("###[DELETE]:co %s was deleted\n",node->name);
#endif
	return;
}

void rand_choose(struct co* head, struct co* candidate, struct co* current) {

	assert(head != NULL);

    int count = 0;

    struct co* temp = head;
    while(temp != NULL) {
        if(temp->status == CO_NEW || temp->status == CO_WAITING) {
	        rand_pool_append(rand_pool_head, temp);
		    count ++;
	    }
		temp = temp->next;
    }
	assert(rand_pool_head != NULL);

#ifdef DEBUG
	printf("### In rand_choose function, there are %d members in rand pool\n", count);
#endif

    //co_count();

	int index = 0;
	srand((unsigned int)time(0));
	if(count != 0) {
       	index = rand() % count + 1;
	}
//	printf("$$$ Index:%d\n", index);
	struct co* pool = rand_pool_head;
	for(int i=0; i < index; i ++) {
	   	pool = pool->brother;
	}
	candidate->brother = pool;

	if(!strcmp(candidate->brother->name, current->name)) {
	    if(count == 2) {
		    index = count + 1 - index;
	        pool = rand_pool_head;
	        for(int i=0; i < index; i ++) {
	   	        pool = pool->brother;
	        }
	        candidate->brother = pool;
		}

		else {
		    index = (index + 1) % count + 1;
//	        printf("$$$ Index Again:%d\n", index);
	        pool = rand_pool_head;
	        for(int i=0; i < index; i ++) {
	   	        pool = pool->brother;
	        }
	        candidate->brother = pool;
		}
	}

	assert(candidate->brother != NULL);
    temp = rand_pool_head->brother;
	rand_pool_head->brother = NULL;
	struct co* old;
	while(temp->brother != NULL) {
		old = temp;
		temp = temp->brother;
		old->brother = NULL;
	}
#ifdef BUG
        	printf("###[CHOOSE]:co %s was chosen | Index:%d\n",candidate->brother->name, index);
#endif
	return;
}

void callback() {

//	printf("HHHHHHHHHHHHHHHHH__co:%s__HHHHHHHHHHHHHHHHHHHHHH\n", current->name);

	current->status = CO_DEAD;
    co_delete(current);    
//	co_count();
//	printf("co %s\n",co_list_head->next->name);
	current_chk();
	printf("%d\tCALL BACK BEFORE LONGJMP\n",__LINE__);
	assert(current->status == CO_DEAD);
    struct co* node = (struct co*)malloc(sizeof(struct co));
	do
	{
		rand_choose(co_list_head, node, current);
	} while(node->brother->status == CO_NEW || !strcmp(node->brother->name, current->name));
	assert(node->brother->status == CO_WAITING);
	//printf("co %s\n",node->brother->name);
	node->brother->status = CO_RUNNING;
	current = node->brother;
	current_chk();
	printf("%d\tCALL BACK AFTER LONGJMP\n",__LINE__);
	longjmp(node->brother->context, 2);

}


struct co *co_start(const char *name, void (*func)(void *), void *arg) {

	assert(name != NULL && func != NULL && arg != NULL);
	struct co *new_co = (struct co*)malloc(sizeof(struct co));

/*
#if __x86_64__
	printf("callback:%p\n", callback);
    stack_display(&new_co->stack[STACK_SIZE-32], 8);	
#else
	printf("callback:%p\n", callback);
    stack_display(&new_co->stack[STACK_SIZE-40], 4);	
#endif
*/
#ifdef TEST_2
	printf("A new space was distributed!\n");
#endif
	new_co->name = (char*)malloc(32*sizeof(char));
	assert(new_co->name != NULL);
	strcpy(new_co->name, name);
#ifdef TEST_2
	printf("The name was copied!\n");
#endif
	new_co->func = func;
#ifdef TEST_2
	printf("The function was pointed!\n");
#endif
	new_co->arg = arg;
#ifdef TEST_2
	printf("The arg was assigned!\n");
#endif
	new_co->status = CO_NEW;
	new_co->next = NULL;
	new_co->brother = NULL;
    
	list_append(co_list_head, new_co);
#ifdef TEST_2
	printf("The new co was appended!\n");
#endif
    assert(co_list_head != NULL);
#ifdef DEBUG
	printf("co %s was created! It's state is %d\n", new_co->name, new_co->status);
#endif
#ifdef BUG
	printf("###[CREATE]:co %s was created\n",new_co->name);
#endif
    return new_co;
}

void co_wait(struct co *co) {
#ifdef BUG
	assert(co->name != NULL);
	printf("###[WAIT]:co %s was waited\n",co->name);
#endif
#ifdef DEBUG
	printf("co %s is to be waited, its state is %d\n", co->name, co->status);
#endif
	if(current == NULL && co->status != CO_DEAD) {
		if(co->status == CO_NEW) {
			co->status = CO_RUNNING;
        	current = co;	
			assert(current == co);
			current_chk();
//*	        printf("%d\tWAIT\n",__LINE__);
			assert(current != NULL);
            
	    	current->func(current->arg);
	    	current->status = CO_DEAD;
	    	current = NULL;
			current_chk();
//*	        printf("%d\tWAIT RETURNED\n",__LINE__);

		co->status = CO_DEAD;
		assert(co != NULL);
#ifdef DEBUG
		printf("co %s was freed\n", co->name);
#endif
        co_delete(co);
#ifdef BUG
	printf("###[FREE]:co %s was freed\n",co->name);
#endif
		free(co);
		return;
		}

		else if(co->status == CO_WAITING) {
#ifdef JMP
			printf("co %s was once run\n", co->name);
#endif
			current = co;
			current_chk();
//*	        printf("%d\tWAIT\n",__LINE__);
			co_yield();
#ifdef DEBUG
		printf("co %s was freed\n", co->name);
#endif
		    co_delete(co);
#ifdef BUG
	printf("###[FREE]:co %s was freed\n",co->name);
#endif
	    	current = NULL;
		    current_chk();
//*	        printf("%d\tWAIT RETURNED\n",__LINE__);
			free(co);
			return;
		}
	}
	else if (current != NULL && co->status != CO_DEAD){
	    current->status = CO_WAITING;
	    struct co *old_current = current;
	    co->status = CO_RUNNING;
	    current = co;
		current_chk();
//*	    printf("%d\tWAIT\n",__LINE__);

	    current->func(current->arg);
	    current->status = CO_DEAD;
	    current = old_current;
		assert(current->name != NULL);
		current_chk();
//*	    printf("%d\tWAIT RETURNED\n",__LINE__);
		current->status = CO_RUNNING;

	    assert(co != NULL);
		co->status = CO_DEAD;
		co_delete(co);
#ifdef BUG
	printf("###[FREE]:co %s was freed\n",co->name);
#endif
	    free(co);
		return;
	}

	else if (co->status == CO_DEAD){
		current = co;
#ifdef BUG
		printf("###[FREE]:co %s was freed\n",co->name);
#endif
		current_chk();
//*	    printf("%d\tWAIT RETURNED\n",__LINE__);
		free(co);
	    return;
	}
	else {
	    assert(0);
	}
}

void co_yield() {
	current_chk();
//*    printf("%d\tYIELD OCCURED\n",__LINE__);
	assert(current != NULL);
#ifdef BUG
	printf("###[YIELD]:co %s was yield\n",current->name);
#endif
	if(current == NULL) {
	    exit(0);
	}
	else {
	    current->status = CO_WAITING;
		//struct co* old_current = current;

		assert(current->context != NULL);
		assert(current != NULL);
	
        int val = setjmp(current->context);
        if (val == 0) {
#ifdef BUG
	printf("###[SETJMP]:co %s's context was saved\n",current->name);
#endif
#ifdef JMP
			printf("A setjmp returned 0, co %s's context was saved\n", current->name);
#endif
            struct co new_co;
			rand_choose(co_list_head, &new_co, current);

			assert(new_co.brother != NULL);
			assert(new_co.brother->status == CO_NEW || new_co.brother->status == CO_WAITING);
			if (new_co.brother->status == CO_NEW) {
				assert(new_co.brother->stack != NULL && new_co.brother->func != NULL && new_co.brother->arg != NULL);
				current = new_co.brother;
		        current_chk();
//*	            printf("%d\tSTACK_SWITCH\n",__LINE__);

#if __x86_64__
	uint64_t num = (uintptr_t)callback;
    for(int i = 0; i < 16; i ++) {
		current->stack[STACK_SIZE-32+i] = num % 256;	    
		num /= 256;
	}	
#else
	uint32_t num = (uintptr_t)callback;
    for(int i = 0; i < 4; i ++) {
		current->stack[STACK_SIZE-40+i] = num % 256;	    
		num /= 256;
	}	
#endif

#ifdef BUG
	printf("###[STACK_SWITCH_CALL]:co %s was put on stack\n",current->name);
#endif
			    stack_switch_call(&new_co.brother->stack[STACK_SIZE-32], new_co.brother->func, (uintptr_t)new_co.brother->arg);
#ifdef DEBUG
				printf("Haha! I am here\n");
#endif
			}

			else {
			   current = new_co.brother;
		       current_chk();
//*	           printf("%d\tLONGJMP BACK BEFORE\n",__LINE__);
			   current->status = CO_RUNNING;
//			   printf("emmm?\n");
#ifdef BUG
	printf("###[LONGJMP]:co %s's context was restored\n",current->name);
#endif
			   longjmp(current->context, 2); 
			}
            
	    }
        else {
		       current_chk();
//*	           printf("%d\tLONGJMP BACK AFTER\n",__LINE__);
//			printf("old_current:%s\n", old_current->name);
		//	current = old_current;
		//	assert(old_current != NULL);

#ifdef JMP
			printf("A longjmp returned 2, co %s's context was restored\n", current->name);
#endif
#ifdef BUG
	printf("###[SETJMP]:co %s's context was restored | State: %d\n",current->name, current->status);
#endif
//	printf("emmm!\n");
//	stack_head_chk();
    			return;
	    }	
	}
}


