#include <NDL.h>
#include <SDL.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

#define keyname_size (sizeof(keyname) / sizeof(char *))
static uint8_t key_state[keyname_size] = {0};

int SDL_PushEvent(SDL_Event *ev) {
  return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
  char buf[64];
  if(NDL_PollEvent(buf, sizeof(buf)) == 0){
    return 0;
  }
  char key[32];
  char value[32];
  sscanf(buf, "%s %s\n", key, value);
  if (strcmp("ku", key) == 0){
    ev->type = SDL_KEYUP;
  }else if (strcmp("kd", key) == 0){
    ev->type = SDL_KEYDOWN;
  }else{
    printf("[SDL_WaitEvent]: unknown event :%s\n", key);
    assert(0);
  }
  for (size_t i = 0; i < keyname_size; i++){
    if(strcmp(keyname[i], value) == 0){
      ev->key.keysym.sym = i;
      printf("%s %s %d\n",key, value, i);
      break;
    }
  }
  return 1;
}

int SDL_WaitEvent(SDL_Event *event) {
  char buf[64];
  while(!NDL_PollEvent(buf, sizeof(buf))){}
  char key[32];
  char value[32];
  sscanf(buf, "%s %s\n", key, value);
  if (strcmp("ku", key) == 0){
    event->type = SDL_KEYUP;
  }else if (strcmp("kd", key) == 0){
    event->type = SDL_KEYDOWN;
  }else{
    printf("[SDL_WaitEvent]: unknown event :%s\n", key);
    assert(0);
  }
  for (size_t i = 0; i < keyname_size; i++){
    if(strcmp(keyname[i], value) == 0){
      event->key.keysym.sym = i;
      printf("%s %s %d\n",key, value, i);
      break;
    }
  }
  return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  return NULL;
}
