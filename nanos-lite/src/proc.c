#include "proc.h"

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used));
static PCB pcb_boot;
PCB *current;

extern void context_uload(PCB *pcb, const char *filename);
extern void context_kload(PCB *pcb, void *entry);
extern void naive_uload(PCB *pcb, const char *filename);

//static int fg_pcb = 1;

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    //Log("Hello World from Nanos-lite for the %dth time!", j);
    j++;
    _yield();
  }
}

void init_proc() {
  //for pa3 start
  // extern void naive_uload(PCB *pcb, const char *filename);
	// naive_uload(NULL, "/bin/init");
  // return;

  // for pa4.1 hello
  context_kload(&pcb[0], (void *)hello_fun);
  //for pa4.1 PAL
  context_uload(&pcb[1], "/bin/init");
  switch_boot_pcb();
  //return;


  return;
}

//static uint32_t count = 0;
_Context* schedule(_Context *prev) {
  //for pa4.1 hellp
  current->tf = prev;

  //for pa4.1 PAL
  current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
  // if(count++ <50){
  //   current = &pcb[fg_pcb];
  // }
  // else{
  //   count = 0;
  //   current = &pcb[0];
  // }
  //Log("schedule");
  current = &pcb[0];
  return current->tf;
}
