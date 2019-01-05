#include "proc.h"
#include "fs.h"
#include "common.h"

#define DEFAULT_ENTRY 0x4000000
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
  // int fd = fs_open(filename, 0, 0);
	// fs_read(fd, (void *)DEFAULT_ENTRY, fs_filesz(fd));
	// fs_close(fd);
  // return DEFAULT_ENTRY;
  int fd = fs_open(filename, 0, 0);
  size_t size = fs_filesz(fd);
  size_t page_number = (size + PGSIZE - 1) / PGSIZE;
  void* p_addr;
  void* v_addr = (void*)DEFAULT_ENTRY;
  for(int i = 0; i < page_number; i ++){
    p_addr = new_page(1);
    _map(&pcb->as, v_addr, p_addr, 0);
    fs_read(fd, p_addr, (((size - i * PGSIZE) < PGSIZE) ? (size - i * PGSIZE) : PGSIZE));
    v_addr += PGSIZE;
  }
  pcb->max_brk = (uintptr_t) v_addr;
  pcb->cur_brk = (uintptr_t) v_addr;
  fs_close(fd);
  Log("loader");
  return DEFAULT_ENTRY;
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
  _protect(&(pcb->as));
  Log("%s", filename);
  uintptr_t entry = loader(pcb, filename);

  _Area stack;
  stack.start = pcb->stack;
  stack.end = stack.start + sizeof(pcb->stack);

  pcb->tf = _ucontext(&pcb->as, stack, stack, (void *)entry, NULL);
}
