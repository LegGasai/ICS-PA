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

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <memory/vaddr.h>
enum {
  TK_NOTYPE = 256, 
  TK_EQ,
  /* TODO: Add more token types */
  TK_DECIMAL,
  TK_HEXADECMAL,
  TK_NEG_DECIMAL,
  TK_NEG_HEXADECMAL,
  TK_REG,
  TK_NEQ,
  TK_AND,
  TK_DEREF,
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
  {"\\-0[xX][A-Fa-f0-9]+", TK_NEG_HEXADECMAL}, // hexadecimal number
  {"\\-[0-9]+", TK_NEG_DECIMAL}, // decimal number
  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"\\-", '-'},         // minus
  {"\\*", '*'},         // mul
  {"\\/", '/'},         // div
  {"\\(", '('},         // left
  {"\\)", ')'},         // right
  {"==", TK_EQ},        // equal
  {"!=", TK_NEQ},        // not equal
  {"&&", TK_AND},        // and 
  {"0[xX][A-Fa-f0-9]+", TK_HEXADECMAL}, // hexadecimal number
  {"[0-9]+", TK_DECIMAL}, // decimal number
  {"\\$\\w+", TK_REG}, // register
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[256] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

// )和数字后的 *以及-
bool is_right_or_num(int type){
  return type == ')' || type == TK_DECIMAL || type == TK_HEXADECMAL || type == TK_REG;
}

static Token create_token(int type,char *substr_start, int substr_len) {
  Token token;
  token.type = type;
  strncpy(token.str, substr_start, substr_len);
  token.str[substr_len] = '\0';  // 确保字符串以 '\0' 结尾
  return token;
}
static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        // Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
        //    i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        switch (rules[i].token_type) {
          case TK_NOTYPE: 
          {
            break;
          }
          case TK_EQ:
          {
            Token token = create_token(TK_EQ, substr_start, substr_len);
            tokens[nr_token++] = token;
            break;
          }
          case TK_NEQ:
          {
            Token token = create_token(TK_NEQ, substr_start, substr_len);
            tokens[nr_token++] = token;
            break;
          }
          case TK_AND:
          {
            Token token = create_token(TK_AND, substr_start, substr_len);
            tokens[nr_token++] = token;
            break;
          }
          case '+':
          {
            Token token = create_token('+', substr_start, substr_len);
            tokens[nr_token++] = token;
            break;
          }
          case '-':
          {
            Token token = create_token('-', substr_start, substr_len);
            tokens[nr_token++] = token;
            break;
          }
          case '*':
          {
            Token token = create_token('*', substr_start, substr_len);
            tokens[nr_token++] = token;
            break;
          }
          case '/':
          {
            Token token = create_token('/', substr_start, substr_len);
            tokens[nr_token++] = token;
            break;
          }
          case '(':
          {
            Token token = create_token('(', substr_start, substr_len);
            tokens[nr_token++] = token;
            break;
          }
          case ')':
          {
            Token token = create_token(')', substr_start, substr_len);
            tokens[nr_token++] = token;
            break;
          }
          case TK_DECIMAL:
          {
            Token token = create_token(TK_DECIMAL, substr_start, substr_len);
            tokens[nr_token++] = token;  
            break;   
          }
          case TK_HEXADECMAL:
          {
            Token token = create_token(TK_HEXADECMAL, substr_start, substr_len);
            tokens[nr_token++] = token;  
            break;  
          }
          case TK_REG:
          {
            Token token = create_token(TK_REG, substr_start, substr_len);
            tokens[nr_token++] = token;  
            break;  
          }
          case TK_NEG_DECIMAL:
          {
            // neg number
            if (nr_token ==0  || !is_right_or_num(tokens[nr_token-1].type)) {
              Token token = create_token(TK_DECIMAL, substr_start, substr_len);
              tokens[nr_token++] = token;  
              break;  
            }else{
              Token token = create_token('-', substr_start, 1);
              tokens[nr_token++] = token; 
              token = create_token(TK_DECIMAL, substr_start + 1, substr_len-1);
              tokens[nr_token++] = token; 
              break;
            }
            
          }
          case TK_NEG_HEXADECMAL:
          {
            // neg number
            if (nr_token == 0 || !is_right_or_num(tokens[nr_token-1].type)) {
              Token token = create_token(TK_HEXADECMAL, substr_start, substr_len);
              tokens[nr_token++] = token;  
              break;  
            }else{
              Token token = create_token('-', substr_start, 1);
              tokens[nr_token++] = token; 
              token = create_token(TK_HEXADECMAL, substr_start + 1, substr_len-1);
              tokens[nr_token++] = token; 
              break;
            }
          }
   
          default: TODO();
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

bool is_opt(int type) {
  return type == '+' || type == '-' || type == '*' || type == '/'
   || type == TK_EQ || type == TK_NEQ || type == TK_AND || type == TK_DEREF;
}

int get_priority(int type) {
  if (type == '+' || type == '-') return 2;
  else if (type == '*' || type == '/') return 3;
  else if (type == TK_EQ || type == TK_NEQ) return 1;
  else if (type == TK_AND) return 0;
  else if (type == TK_DEREF) return 4;
  else panic("Error: Invalid operation!\n");
}

int get_opt_idx(int p, int q){
  int diff = 0;
  int res = -1;
  int res_p = 0x3f3f3f3f;
  for(int i = p ;i <= q; i++){
    if(tokens[i].type == '('){
      diff++;
    }else if(tokens[i].type == ')'){
      diff--;
    }else if(tokens[i].type == TK_DECIMAL || tokens[i].type == TK_HEXADECMAL){
      continue;
    }else if(is_opt(tokens[i].type) && diff == 0){
      if(get_priority(tokens[i].type) <= res_p) {
        res = i;
        res_p = get_priority(tokens[i].type);
      }
    }
  }
  return res;
}



bool check_parentheses(int p, int q) {
  if (p >= q){
    return false;
  }
  if (p >= nr_token || tokens[p].type != '('){
    return false;
  }
  if (q >= nr_token || tokens[q].type != ')'){
    return false;
  }
  // check the number of left parentheses and right parentheses
  int diff = 0;
  for (size_t i = p+1; i <= q-1; i++){
    if(tokens[i].type == '('){
      diff++;
    }else if (tokens[i].type == ')'){
      diff--;
      if(diff < 0) return false;      
    }
  }
  return diff == 0;
}

int eval(int p, int q);

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  for(int i=0; i < nr_token; i++){
    if (tokens[i].type == '*' && (i == 0 || !is_right_or_num(tokens[i - 1].type))) {
      tokens[i].type = TK_DEREF;
    }
    //printf("[DEBUG]:token[%d]: %s, type: %d, len:%lu\n",i,tokens[i].str, tokens[i].type, strlen(tokens[i].str));
  }
  *success = true;
  return (word_t)eval(0, nr_token - 1);
}

int eval(int p, int q){
  //printf("[DEBUG]: p:%d \tq:%d\n",p,q);
  if(p > q){
    // invalid
    return 0;
  }
  if(p == q){
    if (tokens[p].type == TK_DECIMAL)
      return atoi(tokens[q].str);
    else if(tokens[p].type == TK_HEXADECMAL){
      return (int)strtol(tokens[q].str, NULL, 16);
    }else if (tokens[p].type == TK_REG){
      char *reg = tokens[p].str + 1;
      if (strcmp(reg,"pc") == 0){
        return cpu.pc;
      }
      bool s;
      word_t v = isa_reg_str2val(reg, &s);
      if(s){
        return (int)v;
      }
      else{
        panic("Error: cannot read register: %s!\n", tokens[p].str);
      }
    }
  }

  if(check_parentheses(p, q) == true){
    return eval(p+1, q-1);
  }
  // other case: divide into two sub expr
  int idx = get_opt_idx(p, q);
  if (idx < 0){
    panic("Error: Invalid expression p=%d:%s, q=%d:%s!\n", p, tokens[p].str, q, tokens[q].str);
  }
  int l = eval(p, idx-1);
  int r = eval(idx+1, q);
  switch (tokens[idx].type){
    case '+':
      return l+r;
    case '-':
      return l-r;
    case '*':
      return l*r;
    case '/':
      if(r==0){
        panic("Error: divide by zero!\n");
      }
      return l/r;
    case TK_AND:
      return l && r;
    case TK_EQ:
      return l == r;
    case TK_NEQ:
      return l != r;
    case TK_DEREF:
      return vaddr_read((vaddr_t)r, 4);
    default:
      panic("Error: unknown operation!\n");
  }
}
