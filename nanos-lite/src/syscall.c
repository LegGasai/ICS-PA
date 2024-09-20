#include <common.h>
#include "syscall.h"
#include "fs.h"
#include <sys/time.h>
#include <proc.h>

static char* sysname_map[20] = {
  [SYS_exit]        = "SYS_exit",
  [SYS_yield]       = "SYS_yield",
  [SYS_open]        = "SYS_open",
  [SYS_read]        = "SYS_read",
  [SYS_write]       = "SYS_write",
  [SYS_kill]        = "SYS_kill",
  [SYS_getpid]      = "SYS_getpid",
  [SYS_close]       = "SYS_close",
  [SYS_lseek]       = "SYS_lseek",
  [SYS_brk]         = "SYS_brk",
  [SYS_fstat]       = "SYS_fstat",
  [SYS_time]        = "SYS_time",
  [SYS_signal]      = "SYS_signal",
  [SYS_execve]      = "SYS_execve",
  [SYS_fork]        = "SYS_fork",
  [SYS_link]        = "SYS_link",
  [SYS_unlink]      = "SYS_unlink",
  [SYS_wait]        = "SYS_wait",
  [SYS_times]       = "SYS_times",
  [SYS_gettimeofday]= "SYS_gettimeofday"
};


void sys_write(Context *c) {
  
  intptr_t fd = c->GPR2;
  intptr_t buf = c->GPR3;
  size_t count = c->GPR4;
  // int cnt = 0;
  // if(fd == 1 || fd == 2){
  //   for (size_t i = 0; i < count; i++){
  //     putch(*((char*)buf + i));
  //     cnt ++;
  //   }
  //   c->GPRx = cnt;
  // }else{
    
  // }
  c->GPRx = fs_write(fd, (void *)buf, count);
}

void sys_close(Context *c) {
  intptr_t fd = c->GPR2;
  c->GPRx = fs_close(fd);
}

void sys_open(Context *c) {
  c->GPRx = fs_open((char *)c->GPR2, c->GPR3, c->GPR4);
}

void sys_read(Context *c) {
  c->GPRx = fs_read(c->GPR2, (void *)c->GPR3, c->GPR4);
}

void sys_lseek(Context *c) {
  c->GPRx = fs_lseek(c->GPR2, c->GPR3, c->GPR4);
}

void sys_brk(Context *c) {
  c->GPRx = 0;
}

void sys_gettimeofday(Context *c) {
  struct timeval *tv = (struct timeval*)c->GPR2;
  uint64_t time = io_read(AM_TIMER_UPTIME).us;
  tv->tv_usec = time % 1000000;
  tv->tv_sec = time / 1000000;
  c->GPRx = 0;
}

void sys_execve(Context *c) {
  naive_uload(NULL, (char *)c->GPR2);
  c->GPRx = 0;
}

void sys_exit(Context *c) {
  naive_uload(NULL, "/bin/nterm");
  c->GPRx = 0;
}

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;

  switch (a[0]) {
    case SYS_exit :{
      sys_exit(c);
      //printf("[SYS_exit]: do_syscall c->GPRx=%d\n",c->GPRx);
      //halt(c->GPRx);
      break;
    }
    case SYS_yield :{
      //printf("[SYS_yield]: do_syscall c->GPRx=%d\n",c->GPRx);
      yield();
      c->GPRx = 0;
      break;
    }
    case SYS_write: {
      sys_write(c);
      //printf("[SYS_write]: do_syscall c->GPRx=%d\n",c->GPRx);
      break;
    }
    case SYS_brk :{
      sys_brk(c);
      //printf("[SYS_brk]: do_syscall c->GPRx=%d\n",c->GPRx);
      break;
    }
    case SYS_open :{
      sys_open(c);
      //printf("[SYS_open]: do_syscall c->GPRx=%d\n",c->GPRx);
      break;
    }
    case SYS_read :{
      sys_read(c);
      //printf("[SYS_read]: do_syscall c->GPRx=%d\n",c->GPRx);
      break;
    }
    case SYS_close :{
      sys_close(c);
      //printf("[SYS_close]: do_syscall c->GPRx=%d\n",c->GPRx);
      break;
    }
    case SYS_lseek :{
      sys_lseek(c);
      //printf("[SYS_lseek]: do_syscall c->GPRx=%d\n",c->GPRx);
      break;
    }
    case SYS_gettimeofday :{
      sys_gettimeofday(c);
      //printf("[SYS_gettimeofday]: do_syscall c->GPRx=%d\n",c->GPRx);
      break;
    }
    case SYS_execve : {
      sys_execve(c);
      break;
    }
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
  printf("[STrace]: %s, c->GPRx=%d\n",sysname_map[a[0]] ,c->GPRx);
}

