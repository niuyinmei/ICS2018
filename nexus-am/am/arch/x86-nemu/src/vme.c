#include <x86.h>
#include <klib.h>

#define PG_ALIGN __attribute((aligned(PGSIZE)))

static PDE kpdirs[NR_PDE] PG_ALIGN;
static PTE kptabs[PMEM_SIZE / PGSIZE] PG_ALIGN;
static void* (*pgalloc_usr)(size_t);
static void (*pgfree_usr)(void*);

_Area segments[] = {      // Kernel memory mappings
  {.start = (void*)0,          .end = (void*)PMEM_SIZE}
};

#define NR_KSEG_MAP (sizeof(segments) / sizeof(segments[0]))

int _vme_init(void* (*pgalloc_f)(size_t), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  int i;

  // make all PDEs invalid
  for (i = 0; i < NR_PDE; i ++) {
    kpdirs[i] = 0;
  }

  PTE *ptab = kptabs;
  for (i = 0; i < NR_KSEG_MAP; i ++) {
    uint32_t pdir_idx = (uintptr_t)segments[i].start / (PGSIZE * NR_PTE);
    uint32_t pdir_idx_end = (uintptr_t)segments[i].end / (PGSIZE * NR_PTE);
    for (; pdir_idx < pdir_idx_end; pdir_idx ++) {
      // fill PDE
      kpdirs[pdir_idx] = (uintptr_t)ptab | PTE_P;

      // fill PTE
      PTE pte = PGADDR(pdir_idx, 0, 0) | PTE_P;
      PTE pte_end = PGADDR(pdir_idx + 1, 0, 0) | PTE_P;
      for (; pte < pte_end; pte += PGSIZE) {
        *ptab = pte;
        ptab ++;
      }
    }
  }

  set_cr3(kpdirs);
  set_cr0(get_cr0() | CR0_PG);

  return 0;
}

int _protect(_Protect *p) {
  PDE *updir = (PDE*)(pgalloc_usr(1));
  p->pgsize = 4096;
  p->ptr = updir;
  // map kernel space
  for (int i = 0; i < NR_PDE; i ++) {
    updir[i] = kpdirs[i];
  }

  p->area.start = (void*)0x8000000;
  p->area.end = (void*)0xc0000000;
  return 0;
}

void _unprotect(_Protect *p) {
}

static _Protect *cur_as = NULL;
void get_cur_as(_Context *c) {
  c->prot = cur_as;
}

void _switch(_Context *c) {
  set_cr3(c->prot->ptr);
  cur_as = c->prot;
}

int _map(_Protect *p, void *va, void *pa, int mode) {
  uint32_t* ptr = (uint32_t*)p->ptr;
  uint32_t va_shift = (uintptr_t)va >> 2;
  uintptr_t tr = ptr[va_shift];
  if(tr == kpdirs[va_shift]){
    PTE *newtable = (PTE *)(pgalloc_usr(1));
    ptr[va_shift] = (uintptr_t)newtable | PTE_P;
  }
  va_shift = (((uintptr_t)va) & 0x003ff000) >> 12;
  // uint32_t* pgr = (uint32_t*)(tr & 0xfffff000);
  // pgr[va_shift] = (uintptr_t)pa | PTE_P;
  return 0;
}

_Context *_ucontext(_Protect *p, _Area ustack, _Area kstack, void *entry, void *args) {
  _Context* ct = (ustack.end - sizeof(_Context) - 4*sizeof(uintptr_t));

  memset(ct, 0, sizeof(_Context) + 4*sizeof(uintptr_t));
  ct->prot = p;
  ct->eip = (uintptr_t)entry;
  ct->cs = 8;
  ct->eflags = 0x2 | (1<<9);
  *(uintptr_t *)(ustack.end - sizeof(sizeof(uintptr_t))) = 0;
  return ct;
}
