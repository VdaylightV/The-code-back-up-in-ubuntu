#define _GNU_SOURCE
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

int count = 0;

struct node {
    char string[64];
	int depth;
	int pid;
	int ppid;
	struct node* child;
    struct node* peer;	
};
struct node* findpeertail(struct node* Node) {
	struct node* end = (struct node*)Node;
    while(end->peer != NULL) {
	    end = end->peer;
	}
	return (struct node*)end;
}


char space[3] ="  ";

void buildtree(struct node* Node) {
    struct node* temp_node;
    if(Node->child == NULL) {
		for(int i = 0; i < Node->depth; i ++) {
			printf("%s",&space[0]);
		//	printf("%d",Node->depth);
		}
		printf("%s{%d}\n",&(Node->string[0]), Node->pid);
		return;
	}
	else if(Node->child != NULL){
		for(int i = 0; i < Node->depth; i ++) {
			printf("%s",&space[0]);
		//	printf("%d",Node->depth);
		}
		printf("%s{%d}\n",&(Node->string[0]), Node->pid);
	    temp_node = (struct node*)(Node->child);
		while(temp_node != NULL) {
		    buildtree(temp_node);
			temp_node = (struct node*)(temp_node->peer);
		}
	    return;
	}
}


int main() {
	struct node root;
	struct node child1;
	struct node child2;
	struct node child3;
	struct node grachild1;
	struct node grachild2;
	struct node grgrachild1;
	char string1[20] = "Hello, world!";
	char string2[20] = "Hello, future!";
	char string3[20] = "Hello, Hero!";
	char string4[20] = "Hello, Ice!";
	char string5[20] = "Hello, Dream!";
	char string6[20] = "Hello, Angel!";
	char string7[20] = "Hello, Zhui!";
	//root.depth = 0;
	root.child = &child1;
	root.pid = 0;
	//child1.depth = 1;
	child1.pid = 19;
	child1.ppid = 0;
	//child2.depth = 1;
	child2.pid = 29;
	child2.ppid = 0;
	child3.pid = 39;
	//child3.depth = 1;
	child3.ppid = 0;
	//grachild1.depth = 2;
	//grachild2.depth = 2;
	//grgrachild1.depth = 3;
	grachild1.pid = 992;
	grachild2.pid = 21;
	grachild1.ppid = 19;
	grachild2.ppid = 19;
	grgrachild1.pid = 32;
	grgrachild1.ppid = 992;
    strcpy(&(root.string[0]), &string1[0]);	
    strcpy(&(child1.string[0]), &string2[0]);	
    strcpy(&(child2.string[0]), &string3[0]);	
    strcpy(&(child3.string[0]), &string7[0]);	
    strcpy(&(grachild1.string[0]), &string4[0]);	
    strcpy(&(grachild2.string[0]), &string5[0]);	
    strcpy(&(grgrachild1.string[0]), &string6[0]);	
	struct node lib[7];
	struct node lib_lite[3];
	lib[0] = root;
	lib[1] = child1;
	lib[2] = child2;
	lib[3] = child3;
	lib[4] = grachild1;
	lib[5] = grachild2;
	lib[6] = grgrachild1;
	lib_lite[0] = grachild1;
	lib_lite[1] = grachild2;
	lib_lite[2] = grgrachild1;
	/*
	for(int i = 0; i < 3; i ++) {
	    lib_lite[i].depth = 0;
		lib_lite[i].child = NULL;
		lib_lite[i].peer = NULL;
	}
	*/
	struct node* temp = (struct node*)malloc(sizeof(struct node));
	for(int i = 0; i < 7; i ++) {
	    lib[i].depth = 0;
		lib[i].child = NULL;
		lib[i].peer = NULL;
	}
	
	for(int i = 0 ; i < 3; i ++) {
	    for(int j = 0; j < 3; j ++){
		    if(lib_lite[i].child == NULL && lib_lite[j].ppid == lib_lite[i].pid) {
				printf("-----S1\n");
			    lib_lite[i].child = (struct node*)&lib_lite[j];
				lib_lite[j].depth = lib_lite[i].depth + 1;
			}
			else if(lib_lite[i].child != NULL && lib_lite[j].ppid == lib_lite[i].pid){
			  printf("-----S2\n");
			  if(lib_lite[i].child->peer == NULL) {
			      printf("-----S2-1\n");
			      lib_lite[i].child->peer = (struct node*)&lib_lite[j];
			  }
			  else if(lib_lite[i].child->peer != NULL){
			      printf("-----S2-2\n");
                  temp = findpeertail((struct node*)&(lib_lite[i].child->peer));
			      temp->peer = (struct node*)&lib_lite[j];
			  }
			  lib_lite[j].depth = lib_lite[i].depth + 1; 
			}
			else {
			    printf("-----S3\n");
			}
	        printf("i=%d,j=%d,lib[%d].depth=%d, lib[%d].child:%p, lib[%d].depth=%d\n",i,j,i,lib_lite[i].depth,i,lib_lite[i].child,j,lib_lite[j].depth);	
		}
	}
    
	for(int i = 0 ; i < 7; i ++) {
	    for(int j = 0; j < 7; j ++){
		    if(lib[i].child == NULL && lib[j].ppid == lib[i].pid) {
			    lib[i].child = (struct node*)&lib[j];
				lib[j].depth = lib[i].depth + 1;
			}
			else if(lib[i].child != NULL && lib[j].ppid == lib[i].pid){
			  if(lib[i].child->peer == NULL) {
			      lib[i].child->peer = (struct node*)&lib[j];
			  }
			  else if(lib[i].child->peer != NULL){
                  temp = findpeertail((struct node*)&(lib[i].child->peer));
			      temp->peer = (struct node*)&lib[j];
			  }
			  lib[j].depth = lib[i].depth + 1; 
			}
		
		}
	}
	/*
	child1.peer = &child2;
	child1.child = &grachild1;
	grachild1.peer = &grachild2;
	grachild1.child = &grgrachild1;
	child2.child = NULL;
	grachild2.child = NULL;;
	grgrachild1.child = NULL;
	child2.peer = NULL;
	grachild2.peer = NULL;;
	grgrachild1.peer = NULL;
	*/
	//printf("%s\n",&root.string[0]);
	//printf("%s\n",&child1.string[0]);
	//printf("%s\n",&child2.string[0]);
    //buildtree(&grgrachild);
	
    /*	
    for(int i = 0; i < 3; i ++){
	     printf("String:%s, Pid:%d, PPid:%d, depth:%d\n", &lib_lite[i].string[0], lib_lite[i].pid, lib_lite[i].ppid, lib_lite[i].depth);
	}
    */
    

	/*
	temp = (struct node*)&lib[0];
	while(temp != NULL) {
	//	assert(temp->depth < 3);
	    printf("String:%s, Depth:%d, Pid:%d\n",temp->string, temp->depth, temp->pid);
		temp = temp->child;
	}
	*/

	buildtree((struct node*)&(lib[0]));

    return 0;
}
