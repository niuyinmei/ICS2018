#include "common.h"
#include "syscall.h"
#include "proc.h"
#include "fs.h"

//size_t sys_write(int fd, const void *buf, size_t len);
extern void naive_uload(PCB *pcb, const char *filename);

_Context* do_syscall(_Context *c) {
  uintptr_t a[4];
  uintptr_t result = -1;
  a[0] = c->GPR1;
  a[1] = c->GPR2;
	a[2] = c->GPR3;
	a[3] = c->GPR4;
  //Log("%d", a[0]);
  switch (a[0]) {
    case SYS_yield:
			result = 0;
			break;
		case SYS_exit:
      //printf("system exit\n");
      _halt(0);
      //naive_uload(NULL, "/bin/init");
      break;
    case SYS_write:
      result = fs_write(a[1], (void*)a[2], a[3]);
      //printf("write result:%d\n", result);
      break;
    case SYS_brk:
      Log("break");
      result = mm_brk(a[1]);
      break;
    case SYS_open:
      result = fs_open((void *)a[1], a[2], a[3]);
      //printf("open result:%d\n", result);
      break;
    case SYS_close:
      result = fs_close(a[1]);
      break;
    case SYS_read:
      result = fs_read(a[1], (void *)a[2], a[3]);
      //printf("read result:%d\n", result);
      break;
    case SYS_lseek:
  		result = fs_lseek(a[1], a[2], a[3]);
      //printf("lseek result:%d\n", result);
      break;
    case SYS_execve:
  		naive_uload(NULL, (void *)a[1]);
  		break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  c->GPR1 = result;
  return NULL;
}

/* size_t sys_write(int fd, const void *buf, size_t len){
   //Log("SYS_write");
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
 }*/
