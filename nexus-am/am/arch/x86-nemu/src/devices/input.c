#include <am.h>
#include <x86.h>
#include <amdev.h>
#define I8042_DATA_PORT 0x60

unsigned long long press;
size_t input_read(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_INPUT_KBD: {
      _KbdReg *kbd = (_KbdReg *)buf;
      press = inl(I8042_DATA_PORT);
      kbd->keycode = press;
      if(press != _KEY_NONE){
          kbd->keydown = !(kbd->keydown);
      }
      return sizeof(_KbdReg);
    }
  }
  return 0;
}
