#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

#define MAX_LENGTH_OF_EXPR 127

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  uint32_t value;						// original value of the expr 
  char expr[MAX_LENGTH_OF_EXPR + 1];	// the expression
} WP;

/* TODO: Declaration of the functions in watchpoint.c */
WP *new_wp();
void free_wp(int no);
void print_wp();
void check_wp();

#endif
