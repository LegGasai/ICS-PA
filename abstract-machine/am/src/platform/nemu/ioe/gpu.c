#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)
#define N 32
void __am_gpu_init() {
}
void read_width_and_height(int *height, int *width){
  uint32_t w_and_h = inl(VGACTL_ADDR);
  *height = (int)w_and_h & 0xffff;
  *width = w_and_h >> 16;
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  int height;
  int width;
  read_width_and_height(&height, &width);
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = width, .height = height,
    .vmemsz = 0
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
  if (!ctl->sync && (w == 0 || h == 0)) return;
  int W,H;
  read_width_and_height(&H,&W);
  uint32_t *pixels = ctl->pixels;
  int len = (x + w >= W) ? W - x : w;

  for (int j = 0; j < h; j ++, pixels += w) {
    // draw x
    if (y + j < H) {
      // draw y
      for (int i = 0; i < len; i ++) {
        uint32_t p = pixels[i];
        outl(FB_ADDR + ((x + i) + (j + y) * W) * 4, p);
      }
    }
  }
  if (ctl->sync) {
    // need refresh screen
    outl(SYNC_ADDR, 1);
  }

}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
