# KEMOS 開発

## 環境構築

### 必要なツール

- Ubuntu 22.04 LTS（推奨）
- EDK II (edk2-stable202208)
- LLVM/Clang 14
- NASM
- QEMU

### セットアップ手順

#### 1. 必要なパッケージのインストール

```bash
sudo apt update
sudo apt install -y \
    build-essential \
    clang-14 lld-14 llvm-14-dev \
    nasm \
    acpica-tools \
    uuid-dev \
    qemu-system-x86 qemu-utils \
    git \
    python3 python3-distutils \
    dosfstools \
    ovmf
```

#### 2. EDK2のセットアップ

```bash
# EDK2をクローン
cd ~
git clone https://github.com/tianocore/edk2.git
cd edk2
git checkout edk2-stable202208
git submodule update --init

# BaseToolsをビルド
make -C ~/edk2/BaseTools/Source/C

# 環境変数を設定
source ~/edk2/edksetup.sh
```

#### 3. ツールチェーンのダウンロード

```bash
# プロジェクトのdevenvディレクトリを作成
mkdir -p /home/kemo/workspace/os/devenv
cd /home/kemo/workspace/os/devenv

# x86_64-elfツールチェーンをダウンロード・展開
wget https://github.com/uchan-nos/mikanos-build/releases/download/v2.0/x86_64-elf.tar.gz
tar xzf x86_64-elf.tar.gz
rm x86_64-elf.tar.gz
```

#### 4. LoaderPkgをEDK2にリンク

```bash
cd ~/edk2
ln -s /home/kemo/workspace/os/LoaderPkg LoaderPkg
```

#### 5. EDK2ビルド設定

`~/edk2/Conf/target.txt`を以下のように編集：

```
ACTIVE_PLATFORM       = LoaderPkg/LoaderPkg.dsc
TARGET                = DEBUG
TARGET_ARCH           = X64
TOOL_CHAIN_TAG        = CLANG38
```

または、sedコマンドで一括設定：

```bash
cd ~/edk2
sed -i 's/^ACTIVE_PLATFORM.*$/ACTIVE_PLATFORM       = LoaderPkg\/LoaderPkg.dsc/' Conf/target.txt
sed -i 's/^TARGET_ARCH.*$/TARGET_ARCH           = X64/' Conf/target.txt
sed -i 's/^TOOL_CHAIN_TAG.*$/TOOL_CHAIN_TAG        = CLANG38/' Conf/target.txt
```

## ビルド方法

### ブートローダーのビルド

```bash
# EDK2環境変数を設定（毎回必要）
cd ~/edk2
source edksetup.sh

# ビルド
build

# 生成物の確認
ls -lh ~/edk2/Build/LoaderX64/DEBUG_CLANG38/X64/Loader.efi
```

生成されたLoader.efiをプロジェクトディレクトリにコピー：

```bash
cp ~/edk2/Build/LoaderX64/DEBUG_CLANG38/X64/Loader.efi /home/kemo/workspace/os/
```

### カーネルのビルド（未実装）

現在、カーネルのビルドシステムは未整備です。
以下のコマンドで手動コンパイル可能：

```bash
cd /home/kemo/workspace/os
source devenv/buildenv.sh

clang++ --target=x86_64-elf -O2 -Wall -g --std=c++17 \
  -ffreestanding -mno-red-zone -fno-exceptions -fno-rtti \
  -c main.cpp -o main.o

ld.lld --entry KernelMain -z norelro --image-base 0x100000 \
  --static -o kernel.elf main.o
```

## 実行方法

### QEMU上での実行

```bash
cd /home/kemo/workspace/os

# .efiファイルを指定して実行
./scripts/run_qemu.sh Loader.efi

# カーネルも含めて実行する場合（kernel.elfが必要）
./scripts/run_qemu.sh Loader.efi kernel.elf
```

### スクリプトの動作

1. `make_image.sh`: FAT32フォーマットのディスクイメージを作成し、EFI/BOOT/BOOTX64.EFIとしてLoader.efiを配置
2. `run_image.sh`: QEMUを起動してイメージを実行

QEMUオプション：
- メモリ: 1GB
- UEFI BIOS: OVMF（scripts/OVMF_CODE.fd, OVMF_VARS.fd）
- USB: xHCI + マウス + キーボード
- モニタ: stdio

## 開発環境変数

`devenv/buildenv.sh`を読み込むことで、カーネルビルドに必要な環境変数を設定：

```bash
source /home/kemo/workspace/os/devenv/buildenv.sh
```

設定される環境変数：
- `CPPFLAGS`: C++コンパイラフラグ（インクルードパス、マクロ定義）
- `LDFLAGS`: リンカフラグ（ライブラリパス）
