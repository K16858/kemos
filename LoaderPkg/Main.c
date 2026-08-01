#include "Elf.h"
#include "Graphics.h"
#include "MemoryMap.h"
#include "frame_buffer_config.hpp"

#include <Guid/FileInfo.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>
#include <Uefi.h>

void Halt(void) {
  while (1)
    __asm__("hlt");
}

EFI_STATUS OpenRootDir(EFI_HANDLE image_handle, EFI_FILE_PROTOCOL **root) {
  EFI_STATUS status;
  EFI_LOADED_IMAGE_PROTOCOL *loaded_image;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *fs;

  status = gBS->OpenProtocol(image_handle, &gEfiLoadedImageProtocolGuid,
                             (VOID **)&loaded_image, image_handle, NULL,
                             EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
  if (EFI_ERROR(status)) {
    return status;
  }

  status = gBS->OpenProtocol(
      loaded_image->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid,
      (VOID **)&fs, image_handle, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
  if (EFI_ERROR(status)) {
    return status;
  }

  return fs->OpenVolume(fs, root);
}

EFI_STATUS EFIAPI UefiMain(EFI_HANDLE image_handle,
                           EFI_SYSTEM_TABLE *system_table) {
  EFI_STATUS status;

  Print(L"Hello, KEMOS World!\n");

  CHAR8 memmap_buf[4096 * 4];
  struct MemoryMap memmap = {sizeof(memmap_buf), memmap_buf, 0, 0, 0, 0};
  status = GetMemoryMap(&memmap);
  if (EFI_ERROR(status)) {
    Print(L"failed to get memory map: %r\n", status);
    Halt();
  }

  EFI_FILE_PROTOCOL *root_dir;
  status = OpenRootDir(image_handle, &root_dir);
  if (EFI_ERROR(status)) {
    Print(L"failed to open root directory: %r\n", status);
    Halt();
  }

  EFI_FILE_PROTOCOL *memmap_file;
  status = root_dir->Open(
      root_dir, &memmap_file, L"\\memmap",
      EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 0);
  if (EFI_ERROR(status)) {
    Print(L"failed to open file '\\memmap': %r\n", status);
    Print(L"Ignored.\n");
  } else {
    status = SaveMemoryMap(&memmap, memmap_file);
    if (EFI_ERROR(status)) {
      Print(L"failed to save memory map: %r\n", status);
      Halt();
    }
    status = memmap_file->Close(memmap_file);
    if (EFI_ERROR(status)) {
      Print(L"failed to close memory map: %r\n", status);
      Halt();
    }
  }

  EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
  status = OpenGOP(image_handle, &gop);
  if (EFI_ERROR(status)) {
    Print(L"failed to open GOP: %r\n", status);
    Halt();
  }

  Print(L"Resolution: %ux%u, Pixel Format: %s, %u pixels/line\n",
        gop->Mode->Info->HorizontalResolution,
        gop->Mode->Info->VerticalResolution,
        GetPixelFormatUnicode(gop->Mode->Info->PixelFormat),
        gop->Mode->Info->PixelsPerScanLine);
  Print(L"Frame Buffer: 0x%0lx - 0x%0lx, Size: %lu bytes\n",
        gop->Mode->FrameBufferBase,
        gop->Mode->FrameBufferBase + gop->Mode->FrameBufferSize,
        gop->Mode->FrameBufferSize);

  EFI_FILE_PROTOCOL *kernel_file;
  status = root_dir->Open(root_dir, &kernel_file, L"\\kernel.elf",
                          EFI_FILE_MODE_READ, 0);
  if (EFI_ERROR(status)) {
    Print(L"failed to open file '\\kernel.elf': %r\n", status);
    Halt();
  }

  UINTN file_info_size = sizeof(EFI_FILE_INFO) + sizeof(CHAR16) * 12;
  UINT8 file_info_buffer[file_info_size];
  status = kernel_file->GetInfo(kernel_file, &gEfiFileInfoGuid, &file_info_size,
                                file_info_buffer);
  if (EFI_ERROR(status)) {
    Print(L"failed to get file information: %r\n", status);
    Halt();
  }

  EFI_FILE_INFO *file_info = (EFI_FILE_INFO *)file_info_buffer;
  UINTN kernel_file_size = file_info->FileSize;

  VOID *kernel_buffer;
  status = gBS->AllocatePool(EfiLoaderData, kernel_file_size, &kernel_buffer);
  if (EFI_ERROR(status)) {
    Print(L"failed to allocate pool: %r\n", status);
    Halt();
  }

  status = kernel_file->Read(kernel_file, &kernel_file_size, kernel_buffer);
  if (EFI_ERROR(status)) {
    Print(L"error: %r", status);
    Halt();
  }

  Elf64_Ehdr *kernel_ehdr = (Elf64_Ehdr *)kernel_buffer;
  if (kernel_ehdr->e_ident[0] != ELFMAG0 || kernel_ehdr->e_ident[1] != 'E' ||
      kernel_ehdr->e_ident[2] != 'L' || kernel_ehdr->e_ident[3] != 'F') {
    Print(L"Invalid ELF magic: %02x %02x %02x %02x\n", kernel_ehdr->e_ident[0],
          kernel_ehdr->e_ident[1], kernel_ehdr->e_ident[2],
          kernel_ehdr->e_ident[3]);
    Halt();
  }
  if (kernel_ehdr->e_type != ET_EXEC && kernel_ehdr->e_type != ET_DYN) {
    Print(L"not an executable ELF file (e_type=%u)\n", kernel_ehdr->e_type);
    Halt();
  }

  UINT64 kernel_first_addr, kernel_last_addr;
  CalcLoadAdrrRange(kernel_ehdr, &kernel_first_addr, &kernel_last_addr);
  UINTN num_pages = (kernel_last_addr - kernel_first_addr + 0xfff) / 0x1000;
  status = gBS->AllocatePages(AllocateAddress, EfiLoaderData, num_pages,
                              &kernel_first_addr);
  if (EFI_ERROR(status)) {
    Print(L"failed to allocate pages: %r", status);
    Halt();
  }

  CopyLoadSegments(kernel_ehdr);
  Print(L"Kernel: 0x%0lx (%lu bytes)\n", kernel_first_addr, kernel_file_size);

  UINT64 entry_addr = kernel_ehdr->e_entry;
  Print(L"ELF Entry Point: 0x%0lx\n", entry_addr);

  status = gBS->FreePool(kernel_buffer);
  if (EFI_ERROR(status)) {
    Print(L"failed to free pool: %r", status);
    Halt();
  }

  status = kernel_file->Close(kernel_file);
  if (EFI_ERROR(status)) {
    Print(L"failed to close kernel file: %r\n", status);
    Halt();
  }

  // #@@range_begin(exit_bs)
  status = gBS->ExitBootServices(image_handle, memmap.map_key);
  if (EFI_ERROR(status)) {
    status = GetMemoryMap(&memmap);
    if (EFI_ERROR(status)) {
      Print(L"failed to get memory map: %r\n", status);
      Halt();
    }
    status = gBS->ExitBootServices(image_handle, memmap.map_key);
    if (EFI_ERROR(status)) {
      Print(L"Could not exit boot service: %r\n", status);
      Halt();
    }
  }
  // #@@range_end(exit_bs)

  // #@@range_begin(pass_frame_buffer_config)
  struct FrameBufferConfig config = {(UINT8 *)gop->Mode->FrameBufferBase,
                                     gop->Mode->Info->PixelsPerScanLine,
                                     gop->Mode->Info->HorizontalResolution,
                                     gop->Mode->Info->VerticalResolution, 0};
  switch (gop->Mode->Info->PixelFormat) {
  case PixelRedGreenBlueReserved8BitPerColor:
    config.pixel_format = kPixelRGBResv8BitPerColor;
    break;
  case PixelBlueGreenRedReserved8BitPerColor:
    config.pixel_format = kPixelBGRResv8BitPerColor;
    break;
  default:
    Print(L"Unimplemented pixel format: %d\n", gop->Mode->Info->PixelFormat);
    Halt();
  }

  typedef void EntryPointType(const struct FrameBufferConfig *);
  EntryPointType *entry_point = (EntryPointType *)entry_addr;
  entry_point(&config);
  // #@@range_end(pass_frame_buffer_config)

  Print(L"All done\n");

  while (1)
    ;
  return EFI_SUCCESS;
}
