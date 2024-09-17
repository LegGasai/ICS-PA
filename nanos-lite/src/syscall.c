#include <common.h>
#include "syscall.h"

void sys_write(Context *c) {
  
  intptr_t fd = c->GPR2;
  intptr_t buf = c->GPR3;
  size_t count = c->GPR4;
  int cnt = 0;
  if(fd == 1 || fd == 2){
    for (size_t i = 0; i < count; i++){
      putch(*((char*)buf + i));
      cnt ++;
    }
    c->GPRx = cnt;
  }else{
    
  }
}

void sys_brk(Context *c) {
  c->GPRx = 0;
}

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;

  switch (a[0]) {
    case SYS_exit :{
      c->GPRx = 0;
      printf("[SYS_exit]: do_syscall c->GPRx=%d\n",c->GPRx);
      halt(c->GPRx);
      break;
    }
    case SYS_yield :{
      printf("[SYS_yield]: do_syscall c->GPRx=%d\n",c->GPRx);
      yield();
      c->GPRx = 0;
      break;
    }
    case SYS_write: {
      sys_write(c);
      printf("[SYS_write]: do_syscall c->GPRx=%d\n",c->GPRx);
      break;
    }
    case SYS_brk :{
      sys_brk(c);
      printf("[SYS_brk]: do_syscall c->GPRx=%d\n",c->GPRx);
      break;
    }
    case SYS_open :{
      break;
    }
    case SYS_read :{
      break;
    }
    case SYS_close :{
      break;
    }
    case SYS_lseek :{
      break;
    }

    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}

