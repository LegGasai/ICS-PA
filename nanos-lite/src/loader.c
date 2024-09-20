#include <proc.h>
#include <elf.h>
#include <common.h>
#include "fs.h"

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif


static uintptr_t loader(PCB *pcb, const char *filename) {
  int fd = fs_open(filename, 0, 0);
  // TODO();
  Elf_Ehdr ehdr;
  // ramdisk_read(&ehdr, 0, sizeof(Elf_Ehdr));
  fs_lseek(fd, 0, SEEK_SET);
  fs_read(fd, &ehdr, sizeof(Elf_Ehdr));
  assert(*(uint32_t *)ehdr.e_ident == 0x464c457f);
  uint16_t ph_num = ehdr.e_phnum;
  Elf_Phdr phs[ph_num];
  // ramdisk_read(phs, ehdr.e_phoff, sizeof(Elf_Phdr) * ph_num);
  fs_lseek(fd, ehdr.e_phoff, SEEK_SET);
  fs_read(fd, phs, sizeof(Elf_Phdr) * ph_num);
  for (size_t i = 0; i < ph_num; i++) {
    Elf_Phdr ph = phs[i];
    if (ph.p_type == PT_LOAD){
      uint64_t offset = ph.p_offset;
      uint64_t filesize = ph.p_filesz;
      uint64_t memsize = ph.p_memsz;
      // ramdisk_read((void*)ph.p_vaddr, offset, memsize);
      fs_lseek(fd, offset, SEEK_SET);
      fs_read(fd, (void*)ph.p_vaddr, filesize);
      memset((void*)(ph.p_vaddr + ph.p_filesz), 0, (memsize - filesize));
    }
  }
  fs_close(fd);
  return ehdr.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) (); // set the start point of program 
}

