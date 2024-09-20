#define SDL_malloc  malloc
#define SDL_free    free
#define SDL_realloc realloc

#define SDL_STBIMAGE_IMPLEMENTATION
#include "SDL_stbimage.h"

SDL_Surface* IMG_Load_RW(SDL_RWops *src, int freesrc) {
  assert(src->type == RW_TYPE_MEM);
  assert(freesrc == 0);
  return NULL;
}

SDL_Surface* IMG_Load(const char *filename) {
  // FILE *fp = fopen(filename, "r");
  // size_t file_size = (size_t)ftell(fp);

  // char *buf = (char *)malloc(sizeof(char) * file_size);
  // fseek(fp, 0, SEEK_SET);
  // fread(buf, file_size, 1, fp);
  // SDL_Surface *img_surface = STBIMG_LoadFromMemory(buf, file_size);
  // fclose(fp);
  // free(buf);
  // return img_surface;
  FILE *fp = fopen(filename, "rb");  // 使用 "rb" 读取二进制文件，防止文件在某些平台上自动转换换行符
  if (fp == NULL) {
    printf("Failed to open file: %s\n", filename);
    return NULL;
  }

  // 获取文件大小
  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  if (size == -1) {
    printf("Failed to get file size for: %s\n", filename);
    fclose(fp);
    return NULL;
  }
  
  // 分配缓冲区
  fseek(fp, 0, SEEK_SET);
  char *buf = (char *)malloc(size);
  if (buf == NULL) {
    printf("Failed to allocate memory for file: %s\n", filename);
    fclose(fp);
    return NULL;
  }

  // 读取文件到缓冲区
  size_t read_size = fread(buf, 1, size, fp);
  if (read_size != size) {
    printf("Failed to read file: %s\n", filename);
    free(buf);
    fclose(fp);
    return NULL;
  }

  // 从缓冲区加载图片
  SDL_Surface *img_surface = STBIMG_LoadFromMemory(buf, size);
  if (img_surface == NULL) {
    printf("Failed to load image from memory: %s\n", filename);
  }

  // 释放资源
  free(buf);
  fclose(fp);
  
  return img_surface;
}

int IMG_isPNG(SDL_RWops *src) {
  return 0;
}

SDL_Surface* IMG_LoadJPG_RW(SDL_RWops *src) {
  return IMG_Load_RW(src, 0);
}

char *IMG_GetError() {
  return "Navy does not support IMG_GetError()";
}
