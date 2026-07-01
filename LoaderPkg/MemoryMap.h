#pragma once

#include <Protocol/SimpleFileSystem.h>
#include <Uefi.h>

struct MemoryMap {
  UINTN   buffer_size;
  VOID   *buffer;
  UINTN   map_size;
  UINTN   map_key;
  UINTN   descriptor_size;
  UINT32  descriptor_version;
};

EFI_STATUS GetMemoryMap(struct MemoryMap *map);
EFI_STATUS SaveMemoryMap(struct MemoryMap *map, EFI_FILE_PROTOCOL *file);
