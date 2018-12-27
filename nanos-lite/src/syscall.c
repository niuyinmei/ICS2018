#include "common.h"
#include "syscall.h"
#include "proc.h"
#include "fs.h"

extern void naive_uload(PCB *pcb, const char *filename);

_Context* do_syscall(_Context *c) {
  uintptr_t a[4];
  uintptr_t result = -1;
  a[0] = c->GPR1;

  switch (a[0]) {
    case SYS_yield:
			result = 1;
			break;
		// case SYS_exit:
		// 	naive_uload(NULL, NULL);
		// 	break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  c->GPRx = result;
  return NULL;
}
