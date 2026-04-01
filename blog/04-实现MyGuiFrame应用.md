# 实现 MyGuiFrame UEFI 应用

## 一、方案说明

### 1.1 双 edk2 分工

| edk2 | 路径 | 用途 |
|------|------|------|
| 毕设 edk2 | `~/Graduation-project/rpi5-uefi/edk2/` | 编译 RPi5 UEFI 固件 + EFI 程序 |
| 上传 edk2 | `~/edk2/` | 参考（提取必要的包） |

### 1.2 从上传 edk2 复制的包

| 包 | 来源 | 作用 |
|---|------|------|
| **StdLib** | 上传 edk2 | 标准 C 库（未使用，简化为纯 UEFI API） |
| **HelloWorldPkg** | 上传 edk2 | Aarch64MemLib（ARM64 memcpy） + StackCheckLib |
| **StackCheckLib** | 上传 edk2 | 栈保护支持 |

---

## 二、环境准备

### 2.1 复制的包到毕设 edk2

```bash
# 从上传的 edk2 复制到毕设 edk2
cp -r ~/edk2/StdLib ~/Graduation-project/rpi5-uefi/edk2/
cp -r ~/edk2/HelloWorldPkg ~/Graduation-project/rpi5-uefi/edk2/
cp -r ~/edk2/MdePkg/Library/StackCheckLib ~/Graduation-project/rpi5-uefi/edk2/MdePkg/Library/
cp -r ~/edk2/MdePkg/Library/StackCheckFailureHookLibNull ~/Graduation-project/rpi5-uefi/edk2/MdePkg/Library/
```

### 2.2 创建自己的包

```
edk2/
├── MyAppPkg/                    # 新建自己的包
│   ├── MyAppPkg.dec             # 包声明
│   ├── MyAppPkg.dsc             # 包构建配置
│   └── MyGuiFrame/              # MyGuiFrame 应用
│       ├── MyGuiFrame.inf       # 模块定义
│       └── MyGuiFrame.c         # 主程序
└── ...
```

---

## 三、创建 MyAppPkg

### 3.1 包声明文件 (MyAppPkg.dec)

```ini
## @file
#  My Application Package
#
#  Copyright (c) 2026, airzam. All rights reserved.<BR>
#

[Defines]
  DEC_SPECIFICATION              = 0x00010005
  PACKAGE_NAME                   = MyAppPkg
  PACKAGE_GUID                   = 5a8f2e1b-3c4d-5e6f-7a8b-9c0d1e2f3a4b
  PACKAGE_VERSION                = 0.1
```

### 3.2 包构建文件 (MyAppPkg.dsc)

```ini
## @file
#  My Application Package DSC
#
#  Copyright (c) 2026, airzam. All rights reserved.<BR>
#

[Defines]
  PLATFORM_NAME                  = MyAppPkg
  PLATFORM_GUID                  = 5a8f2e1b-3c4d-5e6f-7a8b-9c0d1e2f3a4b
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/MyAppPkg
  SUPPORTED_ARCHITECTURES       = AARCH64
  BUILD_TARGETS                  = DEBUG|RELEASE

[LibraryClasses]
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BasePcdLibNull|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  Aarch64MemLib|HelloWorldPkg/Library/Aarch64MemLib/Aarch64MemLib.inf
  StackCheckLib|MdePkg/Library/StackCheckLib/StackCheckLib.inf
  StackCheckFailureHookLib|MdePkg/Library/StackCheckFailureHookLibNull/StackCheckFailureHookLibNull.inf

[Components]
  MyAppPkg/MyGuiFrame/MyGuiFrame.inf

[BuildOptions]
  GCC:*_*_AARCH64_CC_FLAGS = -fno-builtin -fno-stack-protector
```

**关键配置说明**：
- `-fno-stack-protector`：禁用栈保护，避免与 StackCheckLib 冲突
- `StackCheckLib`：提供 `__stack_chk_guard` 符号

### 3.3 模块定义文件 (MyGuiFrame.inf)

```ini
[Defines]
  INF_VERSION                   = 0x00010006
  BASE_NAME                     = MyGuiFrame
  FILE_GUID                     = a912f198-7f1a-4813-b308-c75db806ec84
  MODULE_TYPE                   = UEFI_APPLICATION
  VERSION_STRING                = 0.1
  ENTRY_POINT                   = UefiMain

[Sources]
  MyGuiFrame.c

[Packages]
  MdePkg/MdePkg.dec
  HelloWorldPkg/HelloWorldPkg.dec

[LibraryClasses]
  UefiApplicationEntryPoint
  UefiLib
  PrintLib
  DebugLib
  BaseMemoryLib
  Aarch64MemLib
```

### 3.4 主程序 (MyGuiFrame.c)

```c
/** @file
    MyGuiFrame - A simple UEFI GUI application

    Copyright (c) 2026, airzam. All rights reserved.<BR>

    THE PROGRAM IS DISTRIBUTED UNDER BSD LICENSE ON AN "AS IS" BASIS.
**/

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>

EFI_STATUS EFIAPI UefiMain(
    IN EFI_HANDLE        ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
)
{
  Print(L"\n");
  Print(L"===========================================\n");
  Print(L"  MyGuiFrame UEFI Application\n");
  Print(L"  Author: airzam\n");
  Print(L"===========================================\n");
  Print(L"\n");
  Print(L"Hello from UEFI!\n");
  Print(L"\n");

  // Wait for key press
  Print(L"Press any key to exit...\n");
  gST->ConIn->Reset(gST->ConIn, FALSE);
  EFI_INPUT_KEY Key;
  while (gST->ConIn->ReadKeyStroke(gST->ConIn, &Key) == EFI_NOT_READY) {
    gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, 0);
  }

  Print(L"Goodbye!\n");
  return EFI_SUCCESS;
}
```

---

## 四、编译步骤

### 4.1 编译 EFI 程序 (MyGuiFrame)

编辑 `edk2/Conf/target.txt`：

```ini
ACTIVE_PLATFORM       = MyAppPkg/MyAppPkg.dsc
TARGET               = DEBUG
TARGET_ARCH          = AARCH64
TOOL_CHAIN_TAG       = GCC5
```

然后编译：

```bash
cd ~/Graduation-project/rpi5-uefi/edk2
export GCC5_AARCH64_PREFIX=aarch64-linux-gnu-
source edksetup.sh
build
```

### 4.2 编译 RPi5 固件

恢复 `edk2/Conf/target.txt`：

```ini
ACTIVE_PLATFORM       = edk2-platforms/Platform/RaspberryPi/RPi5/RPi5.dsc
TARGET               = RELEASE
TARGET_ARCH          = AARCH64
TOOL_CHAIN_TAG       = GCC5
```

然后编译固件：

```bash
cd ~/Graduation-project/rpi5-uefi
./build.sh
```

### 4.3 命令行切换（无需修改 target.txt）

使用 `-p` 参数指定平台，无需修改配置文件：

```bash
cd ~/Graduation-project/rpi5-uefi/edk2
export GCC5_AARCH64_PREFIX=aarch64-linux-gnu-
source edksetup.sh

# 编译 MyGuiFrame
build -a AARCH64 -t GCC5 -b DEBUG -p MyAppPkg/MyAppPkg.dsc

# 编译 RPi5 固件（需在 rpi5-uefi 目录）
cd ../..
./build.sh
```

---

## 五、编译产物

### 5.1 EFI 程序

```
edk2/Build/MyAppPkg/DEBUG_GCC5/AARCH64/MyGuiFrame.efi (16KB)
```

### 5.2 RPi5 固件

```
rpi5-uefi/Build/RPi5/RELEASE_GCC/FV/RPI_EFI.fd
```

## 六、复制到 SD 卡并运行

### 5.1 挂载 SD 卡

```bash
sudo mkdir -p /mnt/sdcard
sudo mount -t vfat /dev/sdb1 /mnt/sdcard
```

### 5.2 复制固件

```bash
sudo cp edk2/Build/MyAppPkg/DEBUG_GCC5/AARCH64/MyGuiFrame.efi /mnt/sdcard/
```

### 5.3 在树莓派上运行

1. 将 SD 卡插入树莓派 5
2. 上电启动 RPi5 UEFI
3. 进入 UEFI Shell
4. 执行：

```
Shell> fs0:
fs0:\> MyGuiFrame.efi
```

---

## 七、依赖关系图

```
MyGuiFrame.efi
├── MyAppPkg (自己编写)
│   └── MyGuiFrame.c
├── HelloWorldPkg (从上传 edk2 复制)
│   ├── Aarch64MemLib (ARM64 memcpy)
│   └── HelloWorldPkg.dec (提供 PCD 定义)
├── MdePkg (毕设 edk2 已有)
│   ├── UefiLib
│   ├── BasePrintLib
│   ├── BaseMemoryLib
│   ├── StackCheckLib (栈保护)
│   └── ...
└── ShellPkg (毕设 edk2 已有)
    └── ShellCEntryLib
```

---

## 八、常见问题

### Q1: 编译报错 "implicit declaration of function 'WaitKey'"

`WaitKey()` 不是标准 UEFI API。使用 `gST->ConIn->ReadKeyStroke()` 替代：

```c
gST->ConIn->Reset(gST->ConIn, FALSE);
EFI_INPUT_KEY Key;
while (gST->ConIn->ReadKeyStroke(gST->ConIn, &Key) == EFI_NOT_READY) {
  gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, 0);
}
```

### Q2: 链接报错 `undefined reference to '__stack_chk_guard'`

需要在 BuildOptions 中禁用栈保护：

```ini
[BuildOptions]
  GCC:*_*_AARCH64_CC_FLAGS = -fno-builtin -fno-stack-protector
```

### Q3: 链接报错 `unsupported relocation`

同样是栈保护问题，添加 `-fno-stack-protector` 解决。

---

## 九、常见问题汇总

### 问题1：库依赖链缺失

**现象**：编译报错 `error 4000: Instance of library class [XXX] is not found`

```
error 4000: Instance of library class [PrintLib] is not found
error 4000: Instance of library class [PcdLib] is not found
error 4000: Instance of library class [DebugLib] is not found
...
```

**原因**：EDK2 的库依赖是自动解析的，但需要 DSC 中显式声明所有依赖的库。

**解决**：在 `MyAppPkg.dsc` 的 `[LibraryClasses]` 中逐步添加缺失的库：

```ini
[LibraryClasses]
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BasePcdLibNull|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  Aarch64MemLib|HelloWorldPkg/Library/Aarch64MemLib/Aarch64MemLib.inf
  StackCheckLib|MdePkg/Library/StackCheckLib/StackCheckLib.inf
  StackCheckFailureHookLib|MdePkg/Library/StackCheckFailureHookLibNull/StackCheckFailureHookLibNull.inf
```

---

### 问题2：链接报错 `undefined reference to '__stack_chk_guard'`

**现象**：
```
undefined reference to `__stack_chk_guard'
undefined reference to `__stack_chk_fail'
```

**原因**：GCC 的栈保护功能需要 `__stack_chk_guard` 符号，但 EDK2 的 BasePrintLib 在某些配置下会触发栈保护，而 BaseTools 提供的库没有这个符号。

**解决**：
1. 添加 `StackCheckLib` 和 `StackCheckFailureHookLib`
2. 在 INF 文件中添加库依赖

---

### 问题3：链接报错 `unsupported relocation`

**现象**：
```
危险的重寻址: unsupported relocation
recompiling with -fPIC
```

**原因**：GCC 的 `-fstack-protector`（栈保护）与 EDK2 的链接方式不兼容。

**解决**：在 `MyAppPkg.dsc` 中添加：

```ini
[BuildOptions]
  GCC:*_*_AARCH64_CC_FLAGS = -fno-builtin -fno-stack-protector
```

---

### 问题4：`WaitKey()` 未定义

**现象**：
```
error: implicit declaration of function 'WaitKey' [-Werror=implicit-function-declaration]
```

**原因**：`WaitKey()` 是 RobinPkg 自己定义的简化函数，不是标准 UEFI API。

**解决**：使用标准的 `gST->ConIn->ReadKeyStroke()`：

```c
#include <Library/UefiBootServicesTableLib.h>

gST->ConIn->Reset(gST->ConIn, FALSE);
EFI_INPUT_KEY Key;
while (gST->ConIn->ReadKeyStroke(gST->ConIn, &Key) == EFI_NOT_READY) {
  gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, 0);
}
```

---

### 问题5：`gST` 和 `gBS` 未声明

**现象**：
```
error: 'gST' undeclared (first use in this function)
error: 'gBS' undeclared (first use in this function)
```

**原因**：使用全局变量 `gST`（系统表）和 `gBS`（启动服务表）需要包含对应的头文件。

**解决**：包含头文件：

```c
#include <Library/UefiBootServicesTableLib.h>
```

这样 `gST` 和 `gBS` 就自动可用了（EDK2 宏定义）。

---

### 问题6：编译报错 `no PLATFORM_VERSION`

**现象**：
```
error 5000: No PLATFORM_VERSION
```

**解决**：在 DSC 文件的 `[Defines]` 部分添加：

```ini
[Defines]
  PLATFORM_NAME                  = MyAppPkg
  PLATFORM_VERSION               = 0.1
  ...
```

---

### 问题7：LTO 与 GCC 版本不兼容

**现象**：
```
-Wno-lto-type-mismatch
relocation R_AARCH64_ADR_PREL_PG_HI21 against symbol `__stack_chk_guard'
```

**原因**：GCC 13 对 LTO（链接时优化）的检查更严格，与旧版 EDK2 配置不完全兼容。

**解决**：禁用栈保护 + 显式声明 LTO 选项：

```ini
[BuildOptions]
  GCC:*_*_AARCH64_CC_FLAGS = -fno-builtin -fno-stack-protector
```

---

## 十、参考

- 上传 edk2：`~/edk2/RobinPkg/Applications/MyGuiFrame/`（完整 GUI 实现）
- 官方 EDK2 文档：https://github.com/tianocore/tianocore.github.io/wiki/EDK-II
