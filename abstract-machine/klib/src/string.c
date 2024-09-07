#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t len = 0;
  if(s == NULL){
    return len;
  }
  while(*s != '\0'){
    len++;
    s++;
  }
  return len;
}

char *strcpy(char *dst, const char *src) {
  if(src == NULL){
    *dst = '\0';
    return dst;
  }
  char *tmp = dst;
  while(*src != '\0'){
    *dst = *src;
    dst++;
    src++;
  }
  *dst = '\0';
  return tmp;
}

char *strncpy(char *dst, const char *src, size_t n) {
  if(src == NULL){
    *dst = '\0';
    return dst;
  }
  char *tmp = dst;
  size_t len = 0;
  while(len < n){
    *dst = *src;
    dst++;
    src++;
  }
  return tmp;
}

char *strcat(char *dst, const char *src) {
  char *tmp = dst;
  size_t offset = strlen(dst);
  while (*src!='\0'){
    *(dst+offset)=*src;
    src++;
    offset++;
  }
  return tmp;
}

int strcmp(const char *s1, const char *s2) {
  while (*s1 != '\0' || *s2 != '\0') {
    int ans = *s1-*s2;
    if(ans!=0){
      return ans;
    }else{
      s1++;
      s2++;
    }
  }
  return 0;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  size_t len = 0;
  while (len < n) {
    int ans = *s1-*s2;
    if(ans!=0){
      return ans;
    }else{
      s1++;
      s2++;
    }
  }
  return 0;
}

void *memset(void *s, int c, size_t n) {
  void *tmp = s;
  for (size_t i = 0; i < n; i++) {
    *(char *)s = c;
    s = (char *)s + 1;
  }
  return tmp;
}

void *memmove(void *dst, const void *src, size_t n) {
  void *tmp = dst;
  char *s_dst = (char*)dst;
  char *s_src = (char*)src;
  if(s_src < s_dst && s_src + n > s_dst){
    for (size_t i = 0; i < n; i++) {
      *(s_dst + n - 1) = *(s_src + n - 1);
      s_dst--;
      s_src--;
    }
  }else{
    for (size_t i = 0; i < n; i++) {
      *(char *)dst = *(char *)src;
      src = (char *)src + 1;
      dst = (char *)dst + 1;
    }
  }

  return tmp;
}

void *memcpy(void *out, const void *in, size_t n) {
  void *tmp = out;
  for (size_t i = 0; i < n; i++) {
    *(char *)out = *(char *)in;
    out = (char *)out + 1;
    in = (char *)in + 1;
  }
  return tmp;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  while (n--) {
    int ans = *(char *)s1-*(char *)s2;
    if(ans!=0){
      return ans;
    }else{
      s1 = (char *)s1 + 1;
      s2 = (char *)s2 + 1;
    }
  }
  return 0;
}

#endif
