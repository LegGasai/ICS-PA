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

#include "sdb.h"

#define NR_WP 32


static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp() {
  WP *p = free_;
  if (p == NULL){
    panic("Error: Exceed maximum number of watchpoints: %d\n",NR_WP);
  }
  
  free_ = free_->next;
  p->next = head;
  head = p;
  return p;
}

void free_wp(WP *wp) {
  WP *temp = head;
  WP *pre = NULL;
  while (temp != NULL) {
    if(temp == wp){
      break;
    }
    pre = temp;
    temp = temp->next;
  }
  if(temp == NULL) {
    panic("Error: cannot free the watchpoint with NO: %d\n",wp->NO);
  }

  if(pre == NULL){
    head = temp->next;
  }else{
    pre->next = temp->next;
  }

  memset(wp->expr, 0, sizeof(wp->expr)); // Initialize expr to empty string
  wp->value = 0; // Initialize value to 0
  wp->next = free_;
  free_ = wp;
}

WP* find_by_no(int no) {
  WP *temp = head;
  while (temp != NULL) {
    if(temp->NO == no){
      return temp;
    }
    temp = temp->next;
  }
  panic("Error: cannot find the watchpoint with NO: %d\n",no);
}

void display_wp() {
  WP *temp = head;
  while (temp != NULL) {
    printf("NO: %-2d\t Expression: %-16s\t Value: %-8d\t0x%08x\n",temp->NO, temp->expr, temp->value, temp->value);
    temp = temp->next;
  }
}

bool check_wp_change() {
  bool is_change = false;
  WP *temp = head;
  while (temp != NULL) {
    bool s;
    word_t new_v = expr(temp->expr, &s);
    if(s){
      if(new_v != temp->value){
        is_change = true;
        printf("WP %-2d : %-16s changed! \nNew Value = %-8d Old Value = %-8d\n", temp->NO, temp->expr, new_v, temp->value);
        temp->value = new_v;
      }
    }else{
      panic("Error: cannot calculate the expression: %s in watchpoint: %d\n", temp->expr, temp->NO);
    }
    temp = temp->next;
  }
  return is_change;
}