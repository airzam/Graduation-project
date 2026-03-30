# 编译树莓派 5 UEFI 固件并烧录 SD 卡

本文记录从上游 worproject/rpi5-uefi 克隆源码、编译生成固件，到将固件烧录到 SD 卡的完整过程。

## 一、克隆上游源码

```bash
cd ~/Graduation-project
git clone --recursive https://github.com/worproject/rpi5-uefi.git
```

注意：`--recursive` 用于克隆所有子模块（arm-trusted-firmware、edk2 等）。

如果克隆超时或子模块克隆失败，可手动修复：

```bash
# 修复单个子模块
cd rpi5-uefi/edk2
git submodule update --init --recursive 子模块路径
```

常见需要修复的子模块：
- `BaseTools/Source/C/BrotliCompress/brotli`
- `CryptoPkg/Library/MbedTlsLib/mbedtls`
- `MdePkg/Library/MipiSysTLib/mipisyst`
- `UnitTestFrameworkPkg/Library/CmockaLib/cmocka`

注：`subhook` 子模块仓库已不存在（404），但该模块仅用于单元测试，不影响固件编译。可通过 Gitee 镜像解决：

```bash
cd rpi5-uefi/edk2/UnitTestFrameworkPkg/Library/SubhookLib
git clone https://gitee.com/fanxingkong/subhook.git subhook
```

## 二、安装编译依赖

```bash
sudo apt update
sudo apt install git gcc g++ build-essential gcc-aarch64-linux-gnu iasl python3-pyelftools uuid-dev
```

验证交叉编译器：

```bash
aarch64-linux-gnu-gcc --version
```

## 三、编译固件

```bash
cd rpi5-uefi
export http_proxy="http://192.168.186.1:7897"  # 如需代理
export https_proxy="http://192.168.186.1:7897"
chmod +x build.sh
./build.sh
```

编译成功后会生成两个文件：
- `RPI_EFI.fd`（UEFI 固件，约 2MB）
- `config.txt`（配置文件）

查看生成的文件：

```bash
ls -lh RPI_EFI.fd config.txt
```

编译输出示例：

```
Fd File Name: RPI_EFI (/home/airzam/Graduation-project/rpi5-uefi/Build/RPi5/RELEASE_GCC/FV/RPI_EFI.fd)
FVMAIN [100%Full] 7115584 total, 7115584 used, 0 free
```

## 四、SD 卡分区与烧录

### 4.1 查看 SD 卡设备

插入 SD 卡后，通过 `lsblk` 查看设备名：

```bash
lsblk -o NAME,SIZE,TYPE,MOUNTPOINT
```

通常显示为 `/dev/sdb`（注意不要选错设备）。

### 4.2 取消挂载

```bash
sudo umount /dev/sdb1 /dev/sdb2
```

### 4.3 分区

UEFI 固件只需要一个 FAT32 分区：

```bash
# 创建 MS-DOS 分区表
sudo parted /dev/sdb --script mklabel msdos

# 创建主分区，使用全部空间
sudo parted /dev/sdb --script mkpart primary fat32 1MiB 100%

# 设置为可启动
sudo parted /dev/sdb --script set 1 boot on
```

### 4.4 格式化 FAT32

```bash
sudo mkfs.fat -F 32 /dev/sdb1
```

### 4.5 挂载并复制文件

```bash
sudo mount /dev/sdb1 /mnt/sdcard
sudo cp RPI_EFI.fd config.txt /mnt/sdcard/
```

验证文件：

```bash
ls -la /mnt/sdcard/
```

输出应包含：
- `RPI_EFI.fd` - UEFI 固件
- `config.txt` - 配置文件

### 4.6 卸载

```bash
sudo umount /mnt/sdcard
```

## 五、启动树莓派 5

将 SD 卡插入树莓派 5，接通电源后：

1. 首先显示二维码屏幕
2. 随后出现树莓派 Logo 和进度条
3. 按 `Esc` 进入固件设置
4. 按 `F1` 启动 UEFI Shell

## 六、常见问题

### 6.1 子模块克隆失败

错误：`fatal: could not read Username for 'https://github.com'`

解决：
- 配置 SSH key 或 HTTPS 代理
- 或手动克隆缺失的子模块

### 6.2 编译缺少依赖

错误：`xxx.h: No such file or directory`

解决：
```bash
sudo apt install gcc-aarch64-linux-gnu iasl python3-pyelftools uuid-dev
```

### 6.3 SD 卡无法挂载

检查：
- SD 卡是否已连接（VMware 中 VM → Removable Devices → Connect）
- 分区表是否正确（`sudo fdisk -l /dev/sdb`）

### 6.4 编译后找不到 .fd 文件

确认编译成功（exit code 0），文件位于：
```bash
ls -lh rpi5-uefi/RPI_EFI.fd
```

## 七、延伸

- [worproject/rpi5-uefi GitHub](https://github.com/worproject/rpi5-uefi)
- [EDK2 官方文档](https://github.com/tianocore/tianocore.github.io/wiki/EDK-II)
- [Raspberry Pi 5 官方文档](https://www.raspberrypi.com/documentation/computers/raspberry-pi-5.html)
