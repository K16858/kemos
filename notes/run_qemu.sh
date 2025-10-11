clang -target x86_64-pc-win32-coff -mno-red-zone -fno-stack-protector -fshort-wchar -Wall -c hello.c
lld-link /subsystem:efi_application /entry:EfiMain /out:hello.efi hello.o

qemu-img create -f raw disk.img 200M
mkfs.fat -n "test" -s 2 -f 2 -R 32 -F 32 disk.img
mkdir -p mnt
sudo mount -o loop disk.img mnt
sudo mkdir -p mnt/EFI/BOOT
sudo cp hello.efi mnt/EFI/BOOT/BOOTX64.EFI
sudo umount mnt

qemu-system-x86_64 -drive if=pflash,file=./my_code.fd -drive if=pflash,file=./my_vars.fd -hda disk.img
