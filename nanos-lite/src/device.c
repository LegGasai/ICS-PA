#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

size_t serial_write(const void *buf, size_t offset, size_t len) {
  for (size_t i = 0; i < len; i++){
    putch(*((char*)buf + i));
  }
  return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  AM_INPUT_KEYBRD_T t = io_read(AM_INPUT_KEYBRD);

  if(t.keycode == AM_KEY_NONE) {
    return 0;
  }
  return sprintf((char *)buf, "%s %s\n", 
      t.keydown ? "kd" : "ku",
      keyname[t.keycode]);
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  AM_GPU_CONFIG_T t = io_read(AM_GPU_CONFIG);
  return sprintf((char *)buf, "WIDTH:%d\nHEIGHT:%d\n", t.width, t.height);
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  // AM_DEVREG(11, GPU_FBDRAW,   WR, int x, y; void *pixels; int w, h; bool sync);
  AM_GPU_CONFIG_T ev = io_read(AM_GPU_CONFIG);
  int width = ev.width;
  //int o_len = len;
  offset /= 4;
  len /= 4;
  int y = offset / width;
  int x = offset - width * y;

  io_write(AM_GPU_FBDRAW, x, y, (void *)buf, len, 1, true);
  return len;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
