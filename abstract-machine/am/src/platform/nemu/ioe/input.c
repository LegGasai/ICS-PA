#include <am.h>
#include <nemu.h>
#include <stdio.h>
#define KEYDOWN_MASK 0x8000


void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  uint32_t key_idx = inl(KBD_ADDR);
  kbd->keydown = key_idx > KEYDOWN_MASK;
  kbd->keycode = key_idx % KEYDOWN_MASK;
}
