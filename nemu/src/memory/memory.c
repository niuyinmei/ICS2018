#include "nemu.h"
#include "device/mmio.h"
#include "memory/mmu.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

paddr_t page_translate(paddr_t addr); //implemented in pa4.2

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
  int mmio_n;
  if ((mmio_n = is_mmio(addr)) != -1) {
    return mmio_read(addr, len, mmio_n);
  }
  else {
    return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
  }
}

void paddr_write(paddr_t addr, uint32_t data, int len) {
  int mmio_n;
	if ((mmio_n = is_mmio(addr)) != -1)
		mmio_write(addr, len, data, mmio_n);
	else
 		memcpy(guest_to_host(addr), &data, len);
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  //across the page boundary
  if(((addr & 0xfff) + len) > 0x1000){
    assert(0);
  }
  else{
    paddr_t paddr = page_translate(addr);
    return paddr_read(paddr, len);
  }

}

void vaddr_write(vaddr_t addr, uint32_t data, int len) {
  //across the page boundary
  if(((addr & 0xfff) + len) > 0x1000){
    assert(0);
  }
  else{
    paddr_t paddr = page_translate(addr);
    return paddr_write(paddr, data, len);
  }
  //paddr_write(addr, data, len);
}

paddr_t page_translate(paddr_t addr){
/*
  physical address: 0x xxxx xxxx xx    xx xxxx xxxx     xxxx xxxx xxxx
                                dir            page             offset
*/
if (cpu.cr0.paging == 0 || cpu.cr0.protect_enable == 0) {
    return addr;
  }
  uint32_t kp = cpu.cr3.val & ~0xfff;     // PDE
  uint32_t idx = addr >> 22;              // dir
  kp = paddr_read(kp + (idx << 2), 4);
  if ((kp & 1) == 0) {
    Log("cpu.cr3.val:%u, addr:%u, kp:%u\n", cpu.cr3.val, addr, kp);
  }
  assert(kp & 1);
  kp &= ~0xfff;
  idx = addr << 10 >> 22;                 // page
  kp = paddr_read(kp + (idx << 2), 4);    // page frame
  if ((kp & 1) == 0) {
    Log("cpu.cr3.val:%u, addr:%u, idx:%u, kp:%u\n", cpu.cr3.val, addr, idx, kp);
  }
  assert(kp & 1);
  kp &= ~0xfff;
  idx = addr & 0xfff;                     // offset
  return kp + idx;
}
