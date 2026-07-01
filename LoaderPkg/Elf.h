#pragma once

#include <Uefi.h>

// ELF64型定義
#pragma pack(1)
typedef struct {
  UINT8  e_ident[16];
  UINT16 e_type;
  UINT16 e_machine;
  UINT32 e_version;
  UINT64 e_entry;
  UINT64 e_phoff;
  UINT64 e_shoff;
  UINT32 e_flags;
  UINT16 e_ehsize;
  UINT16 e_phentsize;
  UINT16 e_phnum;
  UINT16 e_shentsize;
  UINT16 e_shnum;
  UINT16 e_shstrndx;
} Elf64_Ehdr;

typedef struct {
  UINT32 p_type;
  UINT32 p_flags;
  UINT64 p_offset;
  UINT64 p_vaddr;
  UINT64 p_paddr;
  UINT64 p_filesz;
  UINT64 p_memsz;
  UINT64 p_align;
} Elf64_Phdr;

typedef UINT16 Elf64_Half;
#pragma pack()

#define PT_LOAD 1
#define ET_EXEC 2
#define ET_DYN  3
#define ELFMAG0 0x7f

#ifndef UINT64_MAX
#define UINT64_MAX 0xFFFFFFFFFFFFFFFFULL
#endif

void CalcLoadAdrrRange(Elf64_Ehdr *kernel_ehdr, UINT64 *kernel_first_addr,
                       UINT64 *kernel_last_addr);
void CopyLoadSegments(Elf64_Ehdr *ehdr);
