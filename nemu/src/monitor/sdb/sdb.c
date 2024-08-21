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

#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"
#include <memory/vaddr.h>


static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args) {
  uint64_t n = 1;
  if (args != NULL){
    sscanf(args, "%lu", &n);
  }
  if (n < 0) {
    printf("N should be greater than 0!\n");
    return 0;
  }
  cpu_exec(n);
  return 0;
}


static int cmd_info(char *args) {
  char *reg_mode = "r";
  char *monitor_mode = "w";
  if (args == NULL){
    return 0;
  }
  if (strcmp(args, reg_mode) == 0) {
    isa_reg_display();
  }else if (strcmp(args, monitor_mode) == 0) {
    display_wp();
  }else {
    printf("invalid args! info cmd args must be r or w!\n");
  }
  return 0;
}

static int cmd_scan(char *args) {
  char *n = strtok(NULL," ");
  char *expr = strtok(NULL, " ");
  if (n == NULL || expr == NULL) {
    printf("x N EXPR, invalid N or EXPR\n");
  }
  int n_value;
  vaddr_t expr_value;
  if (sscanf(n, "%d", &n_value) != 1 || sscanf(expr, "%x", &expr_value) != 1) {
    printf("x N EXPR, invalid format\n");
    return 0; // 返回错误代码
  }
  for (size_t i = 0; i < n_value; i++)
  {
    word_t v = vaddr_read(expr_value, 4);
    printf("[0x%x]: \t0x%08x \t%d\n",expr_value, v, v);
    expr_value += 4;
  }
  return 0;
}

static int cmd_test(char *args) {
  int right_cnt = 0;
  int total_cnt = 0;
  char *nemu = getenv("NEMU_HOME");
  char temp[256];
  strcpy(temp, nemu);
  strcat(temp, "/tools/gen-expr/input");
  FILE *input_file = fopen(temp, "r");
  if (input_file == NULL) {
    perror("Error opening input file");
    return 1;
  }

  char record[1024];
  unsigned real_val;
  char buf[1024];
 
  // 循环读取每一条记录
  while (true) {
    // 读取一行记录
    if (fgets(record, sizeof(record), input_file) == NULL) {
        perror("Reading input file ending");
        break;
    }

    // 分割记录，获取数字和表达式
    char *token = strtok(record, " ");
    if (token == NULL) {
        printf("Invalid record format\n");
        continue;
    }
    real_val = atoi(token); // 将数字部分转换为整数

    // 处理表达式部分，可能跨越多行
    strcpy(buf, ""); // 清空buf
    while ((token = strtok(NULL, "\n")) != NULL) {
        strcat(buf, token);
        strcat(buf, " "); // 拼接换行后的部分，注意添加空格以分隔多行内容
    }


    bool flag = false;
    unsigned res = expr(buf,&flag);
    if(res == real_val)right_cnt ++;
    else{
      printf("Real Value: %u, Cal Value: %u, Expression: %s\n", real_val, res, buf);
    }
    total_cnt ++;
    // 输出结果
    // printf("Real Value: %u, Cal Value: %u, Expression: %s\n", real_val, res, buf);
 
  }
  printf("Test finished!,the accuracy is %d/%d\n",right_cnt, total_cnt);
  fclose(input_file);

  return 0;
}

static int cmd_p(char *args) {
  char *expression = args;
  if(expression == NULL) {
    printf("Invalid expression to calculate!\n");
    return 0;
  }
  bool s_value = true;
  bool *s = &s_value;
  word_t value = expr(expression, s);
  if(*s) {
    printf("%s = %d\t[0x%08x]\n", expression, value, value);
  }else{
    printf("cannot calculate expression: %s\n",expression);
  }
  return 0;
}

static int cmd_w(char *args) {
  char *exp = args;
  if (exp == NULL){
    printf("Invalid expression to watch!\n");
    return 0;
  }
  bool s;
  word_t v = expr(exp, &s);
  if(s){
    WP *wp = new_wp();
    strncpy(wp->expr, args, strlen(exp));
    wp->value = v;
  }else{
    printf("Cannot calculate expression: %s!\n", args);
  }
  return 0;
}

static int cmd_d(char *args) {
  int n;
  if (args != NULL){
    if(sscanf(args, "%d", &n) != 1){
      printf("Invalid watchpoint NO: %s\n", args);
    }
  }else{
    printf("Invalid watchpoint NO: %s\n", args);
  }
  WP *wp = find_by_no(n);
  free_wp(wp);
  return 0;
}

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  /* TODO: Add more commands */
  { "si", "Single step execution of the program", cmd_si },
  { "info", "Print program status, args: [r]register info, [w]watchpoints info", cmd_info },
  { "x", "Scan and print memory from the given position", cmd_scan },
  { "test", "Test", cmd_test },
  { "p", "Calculation the expression", cmd_p },
  { "w", "Set a watchpoint to the expression", cmd_w },
  { "d", "Delete a watchpoint with the given no.", cmd_d },


};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif
    // printf("%s;%s\n",cmd,args);
    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
