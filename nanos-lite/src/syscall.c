#include "common.h"
#include "syscall.h"
#include "proc.h"
#include "fs.h"
size_t sys_write(int fd, const void *buf, size_t len);

_Context* do_syscall(_Context *c) {
  uintptr_t a[4];
  uintptr_t result = -1;
  a[0] = c->GPR1;
  a[1] = c->GPR2;
	a[2] = c->GPR3;
	a[3] = c->GPR4;
  Log("%d", a[0]);
  switch (a[1]) {
    case SYS_yield:
			result = 0;
			break;
		case SYS_exit:
      _halt(a[1]);
      break;
    case SYS_write:
      Log("get sys write");
      result = sys_write(a[1], (void*)a[2], a[3]);
      break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  c->GPRx = result;
  return NULL;
}

size_t sys_write(int fd, const void *buf, size_t len){
  size_t byteswritten;
  switch (fd) {
    case 1:
    case 2:
      byteswritten = 0;
      while(len--){
        _putc(((char*)buf)[byteswritten]);
        byteswritten++;
      }
      return byteswritten;
      break;
    default:  return 0;
  }
}
