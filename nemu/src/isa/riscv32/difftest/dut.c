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
#include <cpu/difftest.h>
#include "../local-include/reg.h"
#define CHECKDIFF(p) if (ref_r->p != cpu.p) { \
  printf("difftest fail at " #p ", expect %#x got %#x\n", ref_r->p, cpu.p); \
  return false; \
}

bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
  bool flag = true;
  int reg_cnt = MUXDEF(CONFIG_RVE, 16, 32);
  for (size_t i = 0; i < reg_cnt; i++){
    if(ref_r->gpr[i] != cpu.gpr[i]){
      flag = false;
      printf("difftest fail at gpr[%ld], expect 0x%x got 0x%x\n", i, ref_r->gpr[i], cpu.gpr[i]);
      break;
    }
  }
  CHECKDIFF(csrs.mtvec);
  CHECKDIFF(csrs.mstatus);
  CHECKDIFF(csrs.mepc);
  //CHECKDIFF(csrs.mcause);
  if (flag && ref_r->pc == cpu.pc) {
    return true;
  }
  pc = ref_r->pc;
  return false;
}

void isa_difftest_attach() {
}
