#include "common.h"
#include "klib.h"

extern _Context* do_syscall(_Context *c);

static _Context* do_event(_Event e, _Context* c) {
  printf("asd\n");
  switch (e.event) {
    switch (e.event) {
      //case _EVENT_YIELD: printf("System Trap.\n"); break;
      case _EVENT_SYSCALL: do_syscall(c); break;
      default: panic("Unhandled event ID = %d", e.event);
    }

  }

  return NULL;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  _cte_init(do_event);
  Log("Initializing interrupt/exception handler1...");
}
