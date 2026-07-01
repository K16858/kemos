#include "Elf.h"

#include <Library/BaseMemoryLib.h>

void CalcLoadAdrrRange(Elf64_Ehdr *kernel_ehdr, UINT64 *kernel_first_addr,
                       UINT64 *kernel_last_addr) {
  Elf64_Phdr *phdr = (Elf64_Phdr *)((UINT64)kernel_ehdr + kernel_ehdr->e_phoff);
  *kernel_first_addr = UINT64_MAX;
  *kernel_last_addr = 0;
  for (int i = 0; i < kernel_ehdr->e_phnum; i++) {
    if (phdr[i].p_type == PT_LOAD) {
      *kernel_first_addr = MIN(*kernel_first_addr, phdr[i].p_vaddr);
      *kernel_last_addr =
          MAX(*kernel_last_addr, phdr[i].p_vaddr + phdr[i].p_memsz);
    }
  }
}

void CopyLoadSegments(Elf64_Ehdr *ehdr) {
  Elf64_Phdr *phdr = (Elf64_Phdr *)((UINT64)ehdr + ehdr->e_phoff);
  for (Elf64_Half i = 0; i < ehdr->e_phnum; ++i) {
    if (phdr[i].p_type != PT_LOAD)
      continue;

    UINT64 segm_in_file = (UINT64)ehdr + phdr[i].p_offset;
    CopyMem((VOID *)phdr[i].p_vaddr, (VOID *)segm_in_file, phdr[i].p_filesz);

    UINTN remain_bytes = phdr[i].p_memsz - phdr[i].p_filesz;
    SetMem((VOID *)(phdr[i].p_vaddr + phdr[i].p_filesz), remain_bytes, 0);
  }
}
