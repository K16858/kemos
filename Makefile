# KEMOS Makefile
# ビルドからQEMU起動テストまでを一貫して実行

.PHONY: all build loader kernel disk run clean help test

# === 設定 ===
PROJECT_ROOT := $(shell pwd)
EDK2_DIR := $(HOME)/edk2
BUILD_DIR := $(EDK2_DIR)/Build/LoaderX64/DEBUG_CLANG38/X64
LOADER_EFI := $(BUILD_DIR)/Loader.efi
KERNEL_ELF := kernel.elf
DISK_IMG := disk.img
MOUNT_POINT := mnt

# ソースディレクトリ
LOADER_DIR := $(PROJECT_ROOT)/LoaderPkg
KERNEL_DIR := $(PROJECT_ROOT)/kernel
SCRIPTS_DIR := $(PROJECT_ROOT)/scripts

# ソースファイル
LOADER_SRC := $(LOADER_DIR)/Main.c
GRAPHICS_DIR := $(KERNEL_DIR)/graphics
INTERRUPT_DIR := $(KERNEL_DIR)/interrupt
INPUT_DIR := $(KERNEL_DIR)/input
SHELL_DIR := $(KERNEL_DIR)/shell

KERNEL_CXXFLAGS := --target=x86_64-elf -O2 -Wall -g --std=c++17 \
	-ffreestanding -mno-red-zone -fno-exceptions -fno-rtti -I$(KERNEL_DIR)

KERNEL_OBJS := \
	$(KERNEL_DIR)/main.o \
	$(GRAPHICS_DIR)/font.o \
	$(GRAPHICS_DIR)/font/terminus_data.o \
	$(GRAPHICS_DIR)/console.o \
	$(INTERRUPT_DIR)/interrupt.o \
	$(INTERRUPT_DIR)/asmfunc.o \
	$(INTERRUPT_DIR)/pic.o \
	$(INPUT_DIR)/keyboard.o \
	$(INPUT_DIR)/keyboard_queue.o \
	$(INPUT_DIR)/readline.o \
	$(SHELL_DIR)/printk.o \
	$(SHELL_DIR)/shell.o

FONT_BDF := $(GRAPHICS_DIR)/font/ter-u16n.bdf
FONT_BIN := $(GRAPHICS_DIR)/font/terminus.bin
FONT_DATA_CPP := $(GRAPHICS_DIR)/font/terminus_data.cpp

# === デフォルトターゲット ===
all: build

# === ヘルプ ===
help:
	@echo "KEMOS Build System"
	@echo ""
	@echo "Usage:"
	@echo "  make build      - Build bootloader and kernel"
	@echo "  make loader     - Build bootloader only"
	@echo "  make kernel     - Build kernel only"
	@echo "  make disk       - Create disk image"
	@echo "  make run        - Run on QEMU"
	@echo "  make test       - Full test (clean -> build -> disk -> QEMU)"
	@echo "  make clean      - Remove build artifacts"
	@echo "  make help       - Show this help"
	@echo ""
	@echo "Environment:"
	@echo "  EDK2_DIR       = $(EDK2_DIR)"
	@echo "  PROJECT_ROOT   = $(PROJECT_ROOT)"

# === ブートローダービルド ===
loader: $(LOADER_EFI)
	@echo "[DONE] Bootloader build completed"
	@cp $(LOADER_EFI) $(PROJECT_ROOT)/Loader.efi
	@ls -lh $(PROJECT_ROOT)/Loader.efi

$(LOADER_EFI): $(LOADER_SRC) $(LOADER_DIR)/Loader.inf $(LOADER_DIR)/LoaderPkg.dsc
	@echo "[BUILD] Building bootloader..."
	@if [ ! -d $(EDK2_DIR) ]; then \
		echo "[ERROR] EDK2 not found: $(EDK2_DIR)"; \
		exit 1; \
	fi
	@if [ ! -L $(EDK2_DIR)/LoaderPkg ]; then \
		echo "[INFO] Linking LoaderPkg to EDK2..."; \
		ln -s $(LOADER_DIR) $(EDK2_DIR)/LoaderPkg; \
	fi
	@cd $(EDK2_DIR) && \
		bash -c "source edksetup.sh > /dev/null 2>&1 && build" || \
		(echo "[ERROR] Build failed"; exit 1)

# === カーネルビルド ===
kernel: $(KERNEL_ELF)
	@echo "[DONE] Kernel build completed"

$(FONT_BIN) $(FONT_DATA_CPP): $(FONT_BDF) $(GRAPHICS_DIR)/font/bdf2bin.py
	@echo "[BUILD] Converting Terminus BDF..."
	@python3 $(GRAPHICS_DIR)/font/bdf2bin.py $(FONT_BDF) $(FONT_BIN) --cpp $(FONT_DATA_CPP)

$(GRAPHICS_DIR)/font/terminus_data.o: $(FONT_DATA_CPP) devenv/buildenv.sh
	@bash -c "source devenv/buildenv.sh && \
		clang++ \$$CPPFLAGS $(KERNEL_CXXFLAGS) \
		-c $(FONT_DATA_CPP) -o $(GRAPHICS_DIR)/font/terminus_data.o"

$(KERNEL_DIR)/main.o: $(KERNEL_DIR)/main.cpp devenv/buildenv.sh
	@bash -c "source devenv/buildenv.sh && \
		clang++ \$$CPPFLAGS $(KERNEL_CXXFLAGS) \
		-c $(KERNEL_DIR)/main.cpp -o $(KERNEL_DIR)/main.o"

$(GRAPHICS_DIR)/font.o: $(GRAPHICS_DIR)/font.cpp devenv/buildenv.sh
	@bash -c "source devenv/buildenv.sh && \
		clang++ \$$CPPFLAGS $(KERNEL_CXXFLAGS) \
		-c $(GRAPHICS_DIR)/font.cpp -o $(GRAPHICS_DIR)/font.o"

$(GRAPHICS_DIR)/console.o: $(GRAPHICS_DIR)/console.cpp devenv/buildenv.sh
	@bash -c "source devenv/buildenv.sh && \
		clang++ \$$CPPFLAGS $(KERNEL_CXXFLAGS) \
		-c $(GRAPHICS_DIR)/console.cpp -o $(GRAPHICS_DIR)/console.o"

$(INTERRUPT_DIR)/interrupt.o: $(INTERRUPT_DIR)/interrupt.cpp devenv/buildenv.sh
	@bash -c "source devenv/buildenv.sh && \
		clang++ \$$CPPFLAGS $(KERNEL_CXXFLAGS) \
		-c $(INTERRUPT_DIR)/interrupt.cpp -o $(INTERRUPT_DIR)/interrupt.o"

$(INTERRUPT_DIR)/pic.o: $(INTERRUPT_DIR)/pic.cpp devenv/buildenv.sh
	@bash -c "source devenv/buildenv.sh && \
		clang++ \$$CPPFLAGS $(KERNEL_CXXFLAGS) \
		-c $(INTERRUPT_DIR)/pic.cpp -o $(INTERRUPT_DIR)/pic.o"

$(INTERRUPT_DIR)/asmfunc.o: $(INTERRUPT_DIR)/asmfunc.asm
	@nasm -f elf64 -o $(INTERRUPT_DIR)/asmfunc.o $(INTERRUPT_DIR)/asmfunc.asm

$(INPUT_DIR)/keyboard.o: $(INPUT_DIR)/keyboard.cpp devenv/buildenv.sh
	@bash -c "source devenv/buildenv.sh && \
		clang++ \$$CPPFLAGS $(KERNEL_CXXFLAGS) \
		-c $(INPUT_DIR)/keyboard.cpp -o $(INPUT_DIR)/keyboard.o"

$(INPUT_DIR)/keyboard_queue.o: $(INPUT_DIR)/keyboard_queue.cpp devenv/buildenv.sh
	@bash -c "source devenv/buildenv.sh && \
		clang++ \$$CPPFLAGS $(KERNEL_CXXFLAGS) \
		-c $(INPUT_DIR)/keyboard_queue.cpp -o $(INPUT_DIR)/keyboard_queue.o"

$(INPUT_DIR)/readline.o: $(INPUT_DIR)/readline.cpp devenv/buildenv.sh
	@bash -c "source devenv/buildenv.sh && \
		clang++ \$$CPPFLAGS $(KERNEL_CXXFLAGS) \
		-c $(INPUT_DIR)/readline.cpp -o $(INPUT_DIR)/readline.o"

$(SHELL_DIR)/printk.o: $(SHELL_DIR)/printk.cpp devenv/buildenv.sh
	@bash -c "source devenv/buildenv.sh && \
		clang++ \$$CPPFLAGS $(KERNEL_CXXFLAGS) \
		-c $(SHELL_DIR)/printk.cpp -o $(SHELL_DIR)/printk.o"

$(SHELL_DIR)/shell.o: $(SHELL_DIR)/shell.cpp devenv/buildenv.sh
	@bash -c "source devenv/buildenv.sh && \
		clang++ \$$CPPFLAGS $(KERNEL_CXXFLAGS) \
		-c $(SHELL_DIR)/shell.cpp -o $(SHELL_DIR)/shell.o"

$(KERNEL_ELF): $(KERNEL_OBJS) devenv/buildenv.sh
	@echo "[BUILD] Linking kernel..."
	@bash -c "source devenv/buildenv.sh && \
		ld.lld \$$LDFLAGS --entry KernelMain -z norelro --image-base 0x100000 \
		--static -o $(PROJECT_ROOT)/$(KERNEL_ELF) $(KERNEL_OBJS)" || \
		(echo "[ERROR] Kernel build failed"; exit 1)
	@ls -lh $(KERNEL_ELF)

# === 統合ビルド ===
build: loader kernel
	@echo ""
	@echo "[SUCCESS] All build completed"
	@echo "  Bootloader: $(PROJECT_ROOT)/Loader.efi"
	@echo "  Kernel:     $(PROJECT_ROOT)/$(KERNEL_ELF)"

# === ディスクイメージ作成 ===
disk: build $(DISK_IMG)
	@echo "[DONE] Disk image created"

$(DISK_IMG): Loader.efi $(KERNEL_ELF)
	@echo "[BUILD] Creating disk image..."
	@if [ ! -f $(SCRIPTS_DIR)/make_image.sh ]; then \
		echo "[ERROR] Script not found: $(SCRIPTS_DIR)/make_image.sh"; \
		exit 1; \
	fi
	@bash $(SCRIPTS_DIR)/make_image.sh $(DISK_IMG) $(MOUNT_POINT) Loader.efi $(KERNEL_ELF) || \
		(echo "[ERROR] Disk image creation failed"; exit 1)

# === QEMU実行 ===
run: disk
	@echo "[RUN] Starting QEMU..."
	@if [ ! -f $(SCRIPTS_DIR)/run_image.sh ]; then \
		echo "[ERROR] Script not found: $(SCRIPTS_DIR)/run_image.sh"; \
		exit 1; \
	fi
	@bash $(SCRIPTS_DIR)/run_image.sh $(DISK_IMG)

# === 一貫テスト（ビルド→ディスク→QEMU） ===
test: clean build disk
	@echo ""
	@echo "[TEST] Starting integration test..."
	@echo "  1. Clean build completed"
	@echo "  2. Disk image created"
	@echo "  3. Starting QEMU..."
	@echo ""
	@bash $(SCRIPTS_DIR)/run_image.sh $(DISK_IMG)

# === クリーンアップ ===
clean:
	@echo "[CLEAN] Cleaning up..."
	@rm -f Loader.efi $(KERNEL_ELF) $(DISK_IMG)
	@rm -f $(KERNEL_OBJS)
	@rm -f $(FONT_BIN) $(FONT_DATA_CPP)
	@rm -rf $(MOUNT_POINT)
	@if [ -d $(BUILD_DIR) ]; then \
		echo "  Cleaning EDK2 build directory..."; \
		rm -rf $(EDK2_DIR)/Build/LoaderX64; \
	fi
	@echo "[DONE] Cleanup completed"

# === 環境チェック ===
check-env:
	@echo "[CHECK] Checking environment..."
	@echo ""
	@echo "=== Required Tools ==="
	@which clang > /dev/null 2>&1 && echo "  [OK] clang: $$(clang --version | head -1)" || echo "  [NG] clang: NOT FOUND"
	@which ld.lld > /dev/null 2>&1 && echo "  [OK] ld.lld: $$(ld.lld --version | head -1)" || echo "  [NG] ld.lld: NOT FOUND"
	@which qemu-system-x86_64 > /dev/null 2>&1 && echo "  [OK] qemu: $$(qemu-system-x86_64 --version | head -1)" || echo "  [NG] qemu: NOT FOUND"
	@which nasm > /dev/null 2>&1 && echo "  [OK] nasm: $$(nasm -v)" || echo "  [NG] nasm: NOT FOUND"
	@echo ""
	@echo "=== Directories ==="
	@test -d $(EDK2_DIR) && echo "  [OK] EDK2: $(EDK2_DIR)" || echo "  [NG] EDK2: NOT FOUND"
	@test -d devenv/x86_64-elf && echo "  [OK] Toolchain: devenv/x86_64-elf" || echo "  [NG] Toolchain: NOT FOUND"
	@test -f $(SCRIPTS_DIR)/OVMF_CODE.fd && echo "  [OK] OVMF: $(SCRIPTS_DIR)/OVMF_CODE.fd" || echo "  [NG] OVMF: NOT FOUND"
	@echo ""

# === EDK2環境セットアップ補助 ===
setup-edk2:
	@echo "[SETUP] Setting up EDK2 environment..."
	@if [ ! -d $(EDK2_DIR) ]; then \
		echo "  Cloning EDK2..."; \
		cd $(HOME) && git clone https://github.com/tianocore/edk2.git; \
		cd $(EDK2_DIR) && git checkout edk2-stable202208 && git submodule update --init; \
	fi
	@echo "  Building BaseTools..."
	@make -C $(EDK2_DIR)/BaseTools/Source/C
	@echo "[DONE] EDK2 setup completed"
