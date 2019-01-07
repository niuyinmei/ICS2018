#include "proc.h"
#include "fs.h"
#include "common.h"

#define DEFAULT_ENTRY 0x4000000
//#define DEFAULT_ENTRY 0x8048000

extern int fs_open(const char *pathname, int flags, int mode);
extern size_t fs_read(int fd, void *buf, size_t len);
extern size_t fs_write(int fd, const void *buf, size_t len);
extern size_t fs_lseek(int fd, size_t offset, int whence);
extern int fs_close(int fd);
extern size_t fs_filesz(int fd);
extern size_t get_ramdisk_size();

extern size_t ramdisk_read(void *buf, size_t offset, size_t len);
static uintptr_t loader(PCB *pcb, const char *filename) {
  // ramdisk_read((void *)DEFAULT_ENTRY, 0, get_ramdisk_size());
  int fd = fs_open(filename, 0, 0);
	fs_read(fd, (void *)DEFAULT_ENTRY, fs_filesz(fd));
	fs_close(fd);
  return DEFAULT_ENTRY;
  /*int fd = fs_open(filename, 0, 0);
  size_t size = fs_filesz(fd);
  size_t page_cnt = (size + PGSIZE - 1) / PGSIZE;
  void* pa;
  void* va = (void*)DEFAULT_ENTRY;
  for(int i = 0; i < page_cnt; i ++){
    pa = new_page(1);
    _map(&pcb->as, va, pa, 0);
    fs_read(fd, pa, (((size - i * PGSIZE) < PGSIZE) ? (size - i * PGSIZE) : PGSIZE));
    va += PGSIZE;
  }
  pcb->max_brk = (uintptr_t) va;
  pcb->cur_brk = (uintptr_t) va;
  fs_close(fd);
  return DEFAULT_ENTRY;*/
}

void naive_uload(PCB *pcb, const char *filename) {
  //Log("%s", filename);
  uintptr_t entry = loader(pcb, filename);
  ((void(*)())entry) ();
}

void context_kload(PCB *pcb, void *entry) {
  _Area stack;
  stack.start = pcb->stack;
  stack.end = stack.start + sizeof(pcb->stack);
  Log("kload");
  pcb->tf = _kcontext(stack, entry, NULL);
}

void context_uload(PCB *pcb, const char *filename) {
  //_protect(&(pcb->as));
  Log("%s", filename);
  uintptr_t entry = loader(pcb, filename);

  _Area stack;
  stack.start = pcb->stack;
  stack.end = stack.start + sizeof(pcb->stack);

  pcb->tf = _ucontext(&pcb->as, stack, stack, (void *)entry, NULL);
}
