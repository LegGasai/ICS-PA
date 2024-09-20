#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <assert.h> 

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;

uint32_t NDL_GetTicks() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int NDL_PollEvent(char *buf, int len) {
  int fd = open("/dev/events", 0, 0);
  int read_n = read(fd, buf, len);
  close(fd);
  return read_n == 0? 0 : 1;
}

static int canvas_w, canvas_h,canvas_x = 0, canvas_y = 0;

void NDL_OpenCanvas(int *w, int *h) {
  if (*w == 0 && *h == 0) {
    *w = screen_w;
    *h = screen_h;
  }

  canvas_w = *w;
  canvas_h = *h;
  canvas_x = (screen_w - canvas_w) / 2;
  canvas_y = (screen_h - canvas_h) / 2;

  if (getenv("NWM_APP")) {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w; screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1) {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
  }
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
  int fd = open("/dev/fb", 0, 0);
  for (size_t i = 0; i < h && y + i < h; i++){
    // pos and 4Byte every block.
    size_t offset = (screen_w * (canvas_y + y + i) + canvas_x + x) * 4;
    lseek(fd, offset, SEEK_SET);
    // write every row
    write(fd, (pixels + i * w), 4 * (x + w <= canvas_w? w: canvas_w-x));
  }
  close(fd);  
}

void NDL_OpenAudio(int freq, int channels, int samples) {
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
  return 0;
}

int NDL_QueryAudio() {
  return 0;
}

// key:value
static void extract_info(char *str, char *key, int* value){
  char buffer[258];
  int len = 0;
  for (char* c = str; *c; ++c){
    if(*c != ' '){
      buffer[len++] = *c;
    }
  }
  buffer[len] = '\0';

  sscanf(buffer, "%[a-zA-Z]:%d", key, value);
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }
  char buf[128],tag[32];
  int size;
  int fd = open("/proc/dispinfo", 0, 0);
  read(fd, buf, sizeof(buf));
  close(fd);

  char *token = strtok(buf,"\n");
  
  while(token != NULL){
  	extract_info(token, tag, &size);
  	
  	if(strcmp(tag,"WIDTH")==0){
  		screen_w = size;
  	}else if(strcmp(tag, "HEIGHT")==0){
  		screen_h = size;
  	}else{
      printf("Invalid content!\n");
  		assert(0);
  		break;
  	}
  	
  	token = strtok(NULL, "\n");
  	
  }
  printf("[NDL_Init]: w= %d, h= %d\n",screen_w,screen_h);
  return 0;
}

void NDL_Quit() {
}
