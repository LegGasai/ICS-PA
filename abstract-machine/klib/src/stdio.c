#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)


void int_to_str(char *s, int d){
  size_t len = 0;
  char buff[12];
  char *p = buff + 11;
  *p = '\0';
  bool is_negative = d < 0;
  if (is_negative) {
    d = -d;
  }

  if (d == 0){
    *--p = '0';
  }else{
    while (d > 0) {
      int c = d%10;
      d /= 10;
      *--p = c+'0';
      len++;
    }
  }
  if(is_negative){
    *--p = '-';
  }
  while(*p != '\0'){
    *s++ = *p++;
  }
  *s = '\0';
}


int printf(const char *fmt, ...) {
  va_list aq;
  va_start(aq, fmt);
  char out[1024];
  int ans = vsprintf(out, fmt, aq);
  va_end(aq);
  putstr(out);
  return ans;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  size_t ans = 0;
  int d;
  char *s;
  char *s_out = out;
  while (*fmt != '\0') {
    char c = *fmt;
    switch (c) {
    case '%':
      fmt++;
      if(*fmt == 's'){
        s = va_arg(ap, char *);
        while (*s != '\0'){
          *s_out++ = *s++;
          ans++;
        }
      }else if(*fmt == 'd'){
        d = va_arg(ap, int);
        char tmp[12];
        char *p = tmp;
        int_to_str(tmp, d);
        while (*p != '\0'){
          *s_out++ = *p++;
          ans++;
        }
      }
      fmt++;
      break;
    default:
      *s_out++ = *fmt++;
      ans++;
      break;
    }
  }
  *s_out = '\0';
  return ans;
}


int sprintf(char *out, const char *fmt, ...) {
  va_list aq;
  va_start(aq, fmt);
  int ans = vsprintf(out, fmt, aq);
  va_end(aq);
  return ans;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
