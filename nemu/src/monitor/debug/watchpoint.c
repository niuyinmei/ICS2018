#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include "monitor/monitor.h"
#include "cpu/reg.h"


#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

static WP empty_head = {-1, NULL, 0, ""};
static WP empty_free = {-1, wp_pool, 0, ""};


void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = &empty_head;
  free_ = &empty_free;
}

/* TODO: Implement the functionality of watchpoint */
WP *new_wp(){
  if(free_->next == NULL)	assert(0);
  WP *new_node = free_->next;
  free_->next = new_node->next;
  new_node->next = head->next;
  head->next = new_node;
  return new_node;
}

void free_wp(int no){
  WP *prev = head;
  WP *cur = head->next;
  
  while (cur != NULL && cur->NO != no){
    prev = cur;
    cur = cur->next;
  }

  if(cur ==	NULL){
    printf("No watchpoint No. %d.\n", no);
	return;
  }
  prev->next = cur->next;
  cur->next = free_->next;
  free_->next = cur;
}

void print_wp(){
  WP *p = head->next;
  if(p == NULL){
    printf("No watchpoints.\n");
	return;
  }
  printf("No.\tExpression\n");
  while(p != NULL){
    printf("%d\t%s\n", p->NO, p->expr);
	p = p->next;
  }
}

void check_wp(){
  WP *p = head->next;
  while(p != NULL){
    bool success;
	uint32_t cur_value = expr(p->expr, &success);
	if(cur_value != p->value){
	  nemu_state = NEMU_STOP; 
	  printf("Watchpoint %d:\t%s\n", p->NO, p->expr);
	  printf("Old value:\t\t%u\n", p->value);
	  printf("New value:\t\t%u\n", cur_value);
	  p->value = cur_value;
	  return;
	}
	p = p->next;
  }   
}
