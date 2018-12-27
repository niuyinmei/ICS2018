#include "common.h"
#include "klib.h"

extern _Context* do_syscall(_Context *c);

static _Context* do_event(_Event e, _Context* c) {
  printf("%d\n", e.event);
  printf("syscall:%d\n", _EVENT_SYSCALL);
  switch (e.event) {
    case _EVENT_YIELD: printf("System Trap.\n"); break;
    case _EVENT_SYSCALL: do_syscall(c); break;
    default: panic("Unhandled event ID = %d", c->GPR1);
  }

  return NULL;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  _cte_init(do_event);
}
