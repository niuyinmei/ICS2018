#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */
  uint32_t temp1;
  uint32_t temp2;
  vaddr_t jumptarget;

  t0 = cpu.flags;
  rtl_push(&t0);
  t0 = cpu.cs;
  rtl_push(&t0);
  t0 = ret_addr;
  rtl_push(&t0);

  cpu.eflags.IF = 0;

  temp1 = vaddr_read(cpu.idtr.base + 8 * NO, 4);
  temp2 = vaddr_read(cpu.idtr.base * 8 * NO + 4, 4);
  jumptarget = ((temp1 & 0x0000FFFF) | (temp2 & 0xFFFF0000));

  decoding.is_jmp = 1;
  decoding.jmp_eip = jumptarget;
}

void dev_raise_intr() {
  cpu.INTR = true;
}
