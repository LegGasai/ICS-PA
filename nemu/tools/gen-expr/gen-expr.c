/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

static int choose(int n){
  return rand() % n;
}
static void gen_num() {
  int num = choose(100);
  if (num == 0){
    buf[strlen(buf)] = '0';
    return;
  }
  char intStr[12]; 
  sprintf(intStr, "%d", num);
  strcat(buf, intStr);
}
static void gen(char c) {
  buf[strlen(buf)] = c;
}
static void gen_rand_op() {
  int type = choose(4);
  char opt;
  switch (type){
    case 0:
      opt = '+';break;
    case 1:
      opt = '-';break;
    case 2:
      opt = '*';break;
    case 3:
      opt = '/';break;
  }
  gen(opt);
}

static void gen_blank() {
  int len = choose(3);
  for (size_t i = 0; i < len; i++)
  {
    buf[strlen(buf)] = ' ';
  }
}
static void gen_rand_expr(int depth) {
  // buf[0] = '\0';
  if(strlen(buf) > 65535 - 10000 || depth > 8) {
    gen_num();
    return;
  }
  switch (choose(6)) {
    case 0: gen_blank(); gen_num(); gen_blank();  break;
    case 1: case 2: case 3: gen('('); gen_rand_expr(depth + 1); gen(')'); break;
    default: gen_rand_expr(depth + 1); gen_blank(); gen_rand_op(); gen_blank(); gen_rand_expr(depth + 1); break;
  }
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    memset(buf, '\0', sizeof(buf));
    gen_rand_expr(0);

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);
    // treat div-by-zero as error
    int ret = system("gcc -Werror=div-by-zero /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);
    if(ret > 0){
      printf("%u %s\n", result, buf);
    }
    
  }
  return 0;
}
