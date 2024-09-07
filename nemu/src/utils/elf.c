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

#include <common.h>
#include <stdio.h>
#include <elf.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>


#define MAX_FUNCS 1000
typedef struct{
    char name[32];
    Elf32_Addr start_addr;
    Elf32_Addr end_addr;
}  ElfFunc;
ElfFunc elf_list[MAX_FUNCS];
int elf_count =  0;
bool enable_elf = false;
int cur_depth = 0;

typedef struct{
    char name[32];
    char type[8];
    int  depth;
    word_t target;
    word_t pc;
}  FuncInfo;
FuncInfo info_list[MAX_FUNCS];
int info_count = 0;

void display_func_table(){
    if(enable_elf==false){return;}
    for (size_t i = 0; i < elf_count; i++){
        printf("FUNC: name: %16s \t at [ 0x%08x, 0x%08x ]\n", elf_list[i].name, elf_list[i].start_addr, elf_list[i].end_addr);
    }
}

ElfFunc* find_func_by_addr(uint32_t addr) {
    for (size_t i = 0; i < elf_count; i++){
        if (elf_list[i].start_addr <= addr && elf_list[i].end_addr >= addr){
            return &elf_list[i];
        }  
    }
    return NULL;
}

void init_elf(const char *elf_file) {
    if (elf_file == NULL){
       Log("ELF file is not given, ignore!\n");
       return;
    }
    Log("Using ELF file: %s\n",elf_file);
    // open elf file
    int fd = open(elf_file, O_RDONLY);
    Assert(fd >= 0, "Can not open '%s'\n", elf_file);

    struct stat st;
    int fs = fstat(fd, &st);
    if (fs < 0){
        close(fd);
        panic("Can not get file size of '%s'\n", elf_file);
    }

    void *map = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if(map == MAP_FAILED) {
        close(fd);
        panic("Can not mmap elf file '%s'\n", elf_file);
    }
    close(fd);

    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)map;
    if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0) {
        munmap(map, st.st_size);
        panic("Invalid elf file '%s'\n", elf_file);
    }

    Elf32_Shdr *shdr = (Elf32_Shdr *)(map + ehdr->e_shoff);
    Elf32_Shdr *symtab_shdr = NULL;
    Elf32_Shdr *strtab_shdr = NULL;

    for (int i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_type == SHT_SYMTAB) {
            symtab_shdr = &shdr[i];
        } else if (shdr[i].sh_type == SHT_STRTAB && i != ehdr->e_shstrndx) {
            strtab_shdr = &shdr[i];
        }
    }

    if (symtab_shdr == NULL || strtab_shdr == NULL) {
        munmap(map, st.st_size);
        panic("No symbol table or string table found in ELF file '%s'\n", elf_file);
    }

    Elf32_Sym *symtab = (Elf32_Sym *)(map + symtab_shdr->sh_offset);
    char *strtab = (char *)(map + strtab_shdr->sh_offset);


    int symcount = symtab_shdr->sh_size / symtab_shdr->sh_entsize;

    for (size_t i = 0; i < symcount; i++){
        if(ELF32_ST_TYPE(symtab[i].st_info) == STT_FUNC) {
            char *func_name = &strtab[symtab[i].st_name];
            Elf32_Addr start_addr = symtab[i].st_value;
            Elf32_Addr end_addr = symtab[i].st_value + symtab[i].st_size - 1;
            if (elf_count >= MAX_FUNCS){
                printf("Func Table is full!\n");
                break;
            }
            strcpy(elf_list[elf_count].name, func_name);
            elf_list[elf_count].start_addr = start_addr;
            elf_list[elf_count].end_addr = end_addr;
            elf_count ++;
        }
    }
    munmap(map, st.st_size);
    enable_elf = true;
    display_func_table();
}

void func_trace(int type,  int rd, word_t target, word_t pc){
    if(enable_elf == false) {return;}
    if(info_count >= MAX_FUNCS) {
        printf("Func Trace List is full!\n");
        return;
    }
    // func_call
    if (rd == 1){
        ElfFunc* func = find_func_by_addr(target);
        info_list[info_count].depth = cur_depth;
        info_list[info_count].pc = pc;
        info_list[info_count].target = target;
        strcpy(info_list[info_count].type, "call");
        if(func == NULL){
            char *name = "???";
            strcpy(info_list[info_count].name, name);
        }else{
            strcpy(info_list[info_count].name, func->name);
        }
        cur_depth ++;
        info_count ++;
    }

    // func_ret
    if (type == 1 && rd == 0){
        cur_depth--;
        ElfFunc* func = find_func_by_addr(pc);
        info_list[info_count].depth = cur_depth;
        info_list[info_count].pc = pc;
        info_list[info_count].target = target;
        strcpy(info_list[info_count].type, "ret");
        if(func == NULL){
            char *name = "???";
            strcpy(info_list[info_count].name, name);
        }else{
            strcpy(info_list[info_count].name, func->name);
        } 
        info_count ++;
    }
}

void display_func_trace(){
    if(enable_elf == false){return;}
    // TODO: do func call statistics.
    for (size_t i = 0; i < info_count; i++){
        if (strcmp("call", info_list[i].type) == 0){
            printf("0x%08x: ",info_list[i].pc);
            for (size_t j = 0; j < info_list[i].depth; j++){
                printf(" ");
            }
            printf("%s [%s@0x%08x]\n",info_list[i].type, info_list[i].name, info_list[i].target);
        }else if (strcmp("ret", info_list[i].type) == 0){
            printf("0x%08x: ",info_list[i].pc);
            for (size_t j = 0; j < info_list[i].depth; j++){
                printf(" ");
            }
            printf("%s  [%s]\n",info_list[i].type, info_list[i].name);
        }
    }
}