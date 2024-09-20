#include <fs.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
  size_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}
size_t serial_write(const void *buf, size_t offset, size_t len);
size_t events_read(void *buf, size_t offset, size_t len);
size_t dispinfo_read(void *buf, size_t offset, size_t len);
size_t fb_write(const void *buf, size_t offset, size_t len);
/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write},
  [FD_FB] = {"/dev/fb" , 0 , 0, invalid_read, fb_write},
  {"/dev/events", 0, 0, events_read, invalid_write},
  {"/proc/dispinfo", 0, 0, dispinfo_read, invalid_write},

#include "files.h"
};

void init_fs() {
  // TODO: initialize the size of /dev/fb
  AM_GPU_CONFIG_T t = io_read(AM_GPU_CONFIG);
  int w = t.width;
  int h = t.height;
  file_table[FD_FB].size = w * h * sizeof(uint32_t);
}

#define table_size (sizeof(file_table)/sizeof(Finfo))

int fs_open(const char *pathname, int flags, int mode) {
  for (size_t i = 0; i < table_size; i++) {
    if(strcmp(pathname, file_table[i].name) == 0){
      file_table[i].open_offset = 0;
      return i;
    }
  }
  panic("[fs_open] : cannot find file: %s\n", pathname);
}

size_t fs_read(int fd, void *buf, size_t len) {
  assert(fd >=0 && fd < table_size);
  Finfo file_info = file_table[fd];
  size_t start = file_info.disk_offset;
  size_t pos = start + file_info.open_offset;
  size_t end = file_info.disk_offset + file_info.size - 1;
  size_t read_len = len;
  if(pos > end){
    return 0;
  }
  if(pos + read_len > end){
    read_len = end - pos + 1;
  }
  int ret = 0;
  if (file_info.read){
    ret = file_info.read(buf, pos, read_len);
  }else{
    ret = ramdisk_read(buf, pos, read_len);
  }
  file_table[fd].open_offset += ret;
  return ret;
}
size_t fs_write(int fd, const void *buf, size_t len) {
  assert(fd >=0 && fd < table_size);
  Finfo file_info = file_table[fd];
  size_t start = file_info.disk_offset;
  size_t pos = start + file_info.open_offset;
  size_t end = file_info.disk_offset + file_info.size - 1;
  size_t write_len = len;
  if(pos > end){
    return 0;
  }
  if(pos + write_len > end){
    write_len = end - pos + 1;
  }
  int ret = 0;
  if (file_info.write) {
    ret = file_info.write(buf, pos, write_len);
  }else{
    ret = ramdisk_write(buf, pos, write_len);
  }
  file_table[fd].open_offset += ret;
  return ret;
}

size_t fs_lseek(int fd, size_t offset, int whence) {
  if(fd <= 2){
    return 0;
  }
  assert(fd >=0 && fd < table_size);
  Finfo *file_info = &file_table[fd];
  size_t new_offset = 0;
  switch (whence){
    case SEEK_SET:
      new_offset = offset;
      break;
    case SEEK_CUR:
      new_offset = file_info->open_offset + offset;
      break;
    case SEEK_END:
      new_offset = file_info->size + offset; 
      break;
    default:
      panic("[fs_lseek]: invalid whence: %d\n", whence);
  }
  if (new_offset < 0 || new_offset > file_info->size){
    printf("[fs_lseek]: invalid offset: %d, size: %d\n", new_offset, file_info->size);
    return -1;
  }
  file_info->open_offset = new_offset;
  return new_offset;
}


int fs_close(int fd) {
  return 0;
}

