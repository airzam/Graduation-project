# 07 - UEFI安全启动与数字签名实战

> 日期：2026-04-03
> 课题：基于树莓派5的UEFI启动程序设计
> GitHub：https://github.com/airzam/Graduation-project

---

## 1. 研究背景：那些崩溃世界的固件漏洞

### 1.1 真实攻击案例

在过去的十几年里，UEFI/BIOS 安全漏洞导致了多次大规模攻击事件：

| 攻击名称 | 时间 | 目标 | 漏洞利用 | 影响 |
|---------|------|------|---------|------|
| **LoJax** | 2018 | 东欧政府机构 | UEFI固件无签名验证 | 首个UEFI rootkit，重装系统无效 |
| **Mebromi** | 2011 | Windows PC | BIOS无签名 | BIOS rootkit，关闭杀软 |
| **RPi僵尸网络** | 2019 | 公网RPi设备 | SSH弱口令+无代码签名 | 组建挖矿僵尸网络 |
| **BadUSB** | 2014 | 插入未知USB设备 | 固定介质无签名验证 | 键盘模拟攻击 |

### 1.2 LoJax 攻击详解（重点）

LoJax 是首个被公开确认的 UEFI rootkit，由 ESET 安全公司于 2018 年发现：

**攻击过程**：
```
1. 攻击者通过钓鱼邮件或漏洞获取目标机器管理员权限
        ↓
2. 恶意软件尝试写入 UEFI BIOS 固件
        ↓
3. 由于 BIOS 无签名验证，写入成功
        ↓
4. 即使重装系统、换硬盘，恶意软件依然存活
        ↓
5. 攻击者实现持久化控制
```

**防御手段**：启用 UEFI Secure Boot + 固件签名验证

---

## 2. 威胁建模：你的RPi5正在被攻击吗？

### 2.1 当前配置的安全隐患

我们的 RPi5 UEFI 固件当前配置：
```bash
DEFINE SECURE_BOOT_ENABLE = FALSE
```

这意味着：
- **任何** EFI 应用程序都可以执行
- 无需签名验证
- 攻击者可以注入任意代码

### 2.2 攻击面分析

```
攻击路径：

[攻击者]
    │
    ├─► 篡改 GitHub 上的 .efi 源码
    │
    ├─► 入侵你的 PC，修改本地 .efi 文件
    │
    └─► 物理访问 SD 卡，直接替换文件
              │
              ▼
        [你 - 毫不知情]
              │
              ▼
        复制到 SD 卡
              │
              ▼
        RPi5 启动
              │
              ▼
        恶意 .efi 执行 ← 无签名验证，直接运行！
```

### 2.3 攻击链示例（本次毕设演示场景）

```
阶段1：无 Secure Boot
━━━━━━━━━━━━━━━━━━━━━━
1. "攻击者"替换 SerialRecv.efi 为恶意版本
        ↓
2. 你把恶意版本复制到 SD 卡
        ↓
3. RPi5 启动，加载恶意 EFI
        ↓
4. 屏幕上显示假数据，或建立后门
        ↓
5. 攻击成功！⚠️

阶段2：启用 Secure Boot
━━━━━━━━━━━━━━━━━━━━━━
1. 固件启用 SECURE_BOOT_ENABLE = TRUE
        ↓
2. 生成签名密钥对，导入公钥到 db 白名单
        ↓
3. 用私钥给 SerialRecv.efi 签名
        ↓
4. RPi5 启动，验证签名
        ↓
5. 恶意版本：无签名 → 拦截！❌
        ↓
6. 正常版本：有签名 → 执行！✅
```

---

## 3. UEFI安全启动原理

### 3.1 密钥层级（PK/KEK/db/dbx）

UEFI Secure Boot 使用分层密钥架构：

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
                    信任链
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

   PK (Platform Key)        ← 最高权限，RPi5 制造商或用户持有
   │
   ├── 签署 KEK
   │
   └── 启用/禁用 Secure Boot 模式

   KEK (Key Exchange Keys)   ← 可信证书颁发者
   │
   ├── 签署 db 的更新
   │
   └── 签署 dbx 的更新

   db (Signature Database)  ← 白名单
   │
   ├── 可信证书列表
   ├── 可信 EFI 镜像哈希列表
   │
   └── 示例：
       - 树莓派官方 bootloader 证书
       - 你自己的签名证书（SigningKey.pub）

   dbx (Forbidden Database)  ← 黑名单
   │
   ├── 已吊销的证书
   ├── 已知的恶意软件哈希
   │
   └── 示例：
       - 泄露的第三方证书
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

### 3.2 签名验证流程

```
当 EFI 镜像尝试加载时：
━━━━━━━━━━━━━━━━━━━━━━

    ┌─────────────────┐
    │   EFI 镜像       │
    │  (如 SerialRecv.efi)
    └────────┬────────┘
             │
             ▼
    ┌─────────────────┐
    │  提取签名块      │
    │  (PKCS#7 格式)  │
    └────────┬────────┘
             │
             ▼
    ┌─────────────────┐
    │  计算镜像 Hash  │
    │  (SHA-256)     │
    └────────┬────────┘
             │
             ▼
    ┌─────────────────┐
    │  验证签名链     │
    │  签名 → 证书   │
    │  证书 → db     │
    └────────┬────────┘
             │
       ┌─────┴─────┐
       │            │
       ▼            ▼
   ┌──────┐    ┌──────┐
   │  通过 │    │ 拒绝  │
   │ db   │    │ dbx  │
   └──────┘    └──────┘
       │            │
       ▼            ▼
   ┌──────┐    ┌──────┐
   │ 执行 │    │ SECURE│
   │ 镜像 │    │ BOOT  │
   │      │    │VIOLATION│
   └──────┘    └──────┘
```

### 3.3 支持的哈希算法

UEFI Secure Boot 支持多种哈希算法：
- SHA-1（已过时，不推荐）
- **SHA-256**（推荐）
- SHA-384
- SHA-512

---

## 4. RPi5 UEFI安全框架（代码分析）

### 4.1 SECURE_BOOT_ENABLE 开关

文件：`edk2-platforms/Platform/RaspberryPi/RPi5/RPi5.dsc`

```makefile
# 第 33 行
DEFINE SECURE_BOOT_ENABLE = FALSE  # 改为 TRUE 启用

# 启用后自动包含的库和组件：
!if $(SECURE_BOOT_ENABLE) == TRUE
  # 认证变量库 - 处理签名的 UEFI 变量
  AuthVariableLib|SecurityPkg/Library/AuthVariableLib/AuthVariableLib.inf

  # 安全启动变量库
  SecureBootVariableLib|SecurityPkg/Library/SecureBootVariableLib/SecureBootVariableLib.inf

  # 安全启动默认密钥配置
  SecureBootDefaultKeysDxe|SecurityPkg/VariableAuthenticated/SecureBootDefaultKeysDxe/SecureBootDefaultKeysDxe.inf

  # 安全启动配置 UI
  SecureBootConfigDxe|SecurityPkg/Universal/SecureBoot/SecureBootConfigDxe/SecureBootConfigDxe.inf

  # TPM 测量（用于日志）
  TpmMeasurementLib|SecurityPkg/Library/DxeTpmMeasurementLib/DxeTpmMeasurementLib.inf
!endif
```

### 4.2 镜像验证策略

当 SECURE_BOOT_ENABLE = TRUE 时，会强制验证所有 EFI 镜像：

```makefile
# DSC 第 365-370 行
!if $(SECURE_BOOT_ENABLE) == TRUE
  # 覆盖默认值为严格模式
  gEfiSecurityPkgTokenSpaceGuid.PcdOptionRomImageVerificationPolicy|0x04
  gEfiSecurityPkgTokenSpaceGuid.PcdFixedMediaImageVerificationPolicy|0x04
  gEfiSecurityPkgTokenSpaceGuid.PcdRemovableMediaImageVerificationPolicy|0x04
!endif
```

**策略值含义**：
| 值 | 策略 | 描述 |
|----|------|------|
| 0x00 | DEVICE_PATH | 仅验证设备路径 |
| 0x01 | CALLOUT | 仅调用钩子 |
| 0x02 | NONE | 不验证 |
| 0x03 | SIGNATURE | 验证签名 |
| **0x04** | FULL | 完全验证（签名 + 哈希）|

### 4.3 DxeImageVerificationLib 分析

这是实际执行签名验证的库，位于 `SecurityPkg/Library/DxeImageVerificationLib/`：

**关键函数**：
- `DxeImageVerificationHandler()` - 主入口
- `HashPeImage()` - 计算 PE/COFF 镜像的哈希
- `IsAllowedByDb()` - 检查是否在白名单
- `IsForbiddenByDbx()` - 检查是否在黑名单
- `VerifyAuthVariable()` - 验证 AuthVariable

---

## 5. 攻击演示：故意被攻击以展示危害

### 5.1 恶意EFI程序设计（SerialRecvMalicious）

为了演示攻击的危害，我创建了一个**无害的**恶意版本演示程序：

**源码位置**：`rpi5-uefi/edk2/SerialPkg/SerialRecvMalicious/`

**功能**：
1. 显示红色警告屏幕
2. 发送伪造的传感器数据到串口
3. 模拟攻击日志写入
4. 展示完整的"受害"过程

**核心代码片段**：
```c
// 显示警告
Print(L"  ██╗  ██╗ ██████╗ ██╗     ██╗      █████╗ ██╗  ██╗███████╗\n");
Print(L"  ██║  ██║██╔═══██╗██║     ██║     ██╔══██╗╚██╗██╔╝██╔════╝\n");
Print(L"  ███████║██║   ██║██║     ██║     ███████║ ╚███╔╝ █████╗\n");
Print(L"  ██╔══██║██║   ██║██║     ██║     ██╔══██║ ██╔██╗ ██╔══╝\n");
Print(L"  ██║  ██║╚██████╔╝███████╗███████╗██║  ██║██╔╝ ██╗███████╗\n");

// 发送假数据
CHAR8 *fakeReadings[] = {
    "FAKE_SENSOR: Temp=9999.9C, Humidity=99%, Pressure=0hPa\r\n",
    "FAKE_SENSOR: CO2=9999ppm, PM2.5=9999ug/m3\r\n",
    // ...
};
```

**无害设计**：
- 只显示警告，不真正破坏系统
- 发送的是明显的假数据（温度9999°C）
- 可以随时 ESC 退出

### 5.2 演示流程

#### 阶段1：未启用 Secure Boot（攻击成功）

```
1. 当前固件：SECURE_BOOT_ENABLE = FALSE
2. 编译 SerialRecv.efi（正常版本）
3. 复制到 SD 卡 → 运行正常 ✅

4. "攻击者"替换为 SerialRecvMalicious.efi
5. 复制到 SD 卡
6. 运行 → 显示警告+假数据 ⚠️
7. 攻击成功演示完毕
```

#### 阶段2：启用 Secure Boot（攻击被拦截）

```
1. 修改 SECURE_BOOT_ENABLE = TRUE
2. 重新编译固件
3. 烧录 SD 卡

4. 进入 UEFI Setup → 配置 Secure Boot
   - 导入 KEKKey.der → KEK
   - 导入 SigningKey.pub → db

5. 用私钥签名 SerialRecv.efi
   $ python3 SignTool.py --input SerialRecv.efi \
                         --output SerialRecv.signed.efi \
                         --key keys/SigningKey.pem \
                         --cert keys/SigningKey.pub

6. 复制 SerialRecv.signed.efi 到 SD 卡
7. 运行 → 正常执行 ✅

8. 尝试运行未签名的 SerialRecvMalicious.efi
9. 运行 → 显示 "SECURE BOOT VIOLATION" ❌
10. 攻击被阻止！
```

---

## 6. 密钥管理工具设计与实现

### 6.1 工具目录

```
rpi5-uefi/edk2/HostTools/
├── README.md           # 使用说明
├── GenKeys.py          # 密钥生成工具
├── SignTool.py         # EFI 签名工具
└── VerifyTool.py       # 签名验证工具（TODO）
```

### 6.2 GenKeys.py - 密钥生成

**功能**：生成 RSA-2048/4096 密钥对

**使用方法**：
```bash
cd HostTools
python3 GenKeys.py --key-dir ./keys --key-size 2048
```

**生成的文件**：
| 文件 | 用途 |
|------|------|
| `PlatformKey.pem` | PK 私钥（最高权限） |
| `PlatformKey.pub` | PK 公钥 |
| `KEKKey.pem` | KEK 私钥 |
| `KEKKey.pub` | KEK 公钥 |
| `SigningKey.pem` | 签名私钥（签署 .efi）|
| `SigningKey.pub` | 签名公钥（导入 UEFI db）|
| `*.der` | DER 格式（UEFI 直接导入）|

**核心代码**：
```python
from cryptography.hazmat.primitives.asymmetric import rsa

def generate_rsa_key(key_size=2048):
    private_key = rsa.generate_private_key(
        public_exponent=65537,
        key_size=key_size,
        backend=default_backend()
    )
    return private_key
```

### 6.3 SignTool.py - EFI 签名

**功能**：对 PE/COFF 镜像进行签名

**使用方法**：
```bash
# 签名 EFI 镜像
python3 SignTool.py \
    --input SerialRecv.efi \
    --output SerialRecv.signed.efi \
    --key keys/SigningKey.pem \
    --cert keys/SigningKey.pub

# 计算 PE Hash（用于 UEFI db 白名单）
python3 SignTool.py --hash --input SerialRecv.efi

# 验证签名
python3 SignTool.py --verify \
    --input SerialRecv.signed.efi \
    --cert keys/SigningKey.pub
```

**PE Hash 计算原理**：
```python
def calculate_pe_image_hash(pe_file):
    """
    计算 UEFI 认证所需的 PE 镜像哈希

    1. 解析 DOS/PE 头
    2. 找到安全目录位置
    3. 对 DOS 头到可选头之间的区域计算 SHA-256
    4. 返回哈希值
    """
    # ... PE 结构解析 ...
    sha256 = hashlib.sha256()
    sha256.update(pe_data[:hash_end])
    return sha256.digest()
```

### 6.4 密钥导入 UEFI Setup

```
UEFI Setup 主界面
    │
    ▼
Device Manager
    │
    ▼
Secure Boot Configuration
    │
    ├── Secure Boot Enable ──────► [TRUE]
    │
    ├── Secure Boot Mode ────────► [Custom]
    │
    └── PK/KEK/db Options
          │
          ├── KEK Options ───────► 导入 KEKKey.der
          │
          └── db Options ─────────► 导入 SigningKey.der
```

---

## 7. 实战：启用RPi5安全启动

### 7.1 修改固件配置

**步骤1**：修改 RPi5.dsc

```bash
# 文件：edk2-platforms/Platform/RaspberryPi/RPi5/RPi5.dsc
# 第 33 行

# 修改前
DEFINE SECURE_BOOT_ENABLE      = FALSE

# 修改后
DEFINE SECURE_BOOT_ENABLE      = TRUE
```

**步骤2**：重新编译固件

> **注意**：启用 Secure Boot 后固件会变大（约增加 48KB），需要修改 FDF 文件调整分区大小。

**2.1 修改 FDF 文件**（如遇到编译错误 `FV image size exceeds`）

文件：`edk2-platforms/Platform/RaspberryPi/RPi5/RPi5.fdf`

需要修改的内容：

```makefile
# FD 大小调整（0x001f0000 → 0x00200000）
Size          = 0x00200000|gArmTokenSpaceGuid.PcdFdSize
NumBlocks     = 0x200

# FV 大小调整（0x001b0000 → 0x001c0000）
0x00020000|0x001c0000
gArmTokenSpaceGuid.PcdFvBaseAddress|gArmTokenSpaceGuid.PcdFvSize
FV = FVMAIN_COMPACT

# 变量存储区偏移调整
# NV_VARIABLE_STORE: 0x001d0000 → 0x001e0000
# NV_EVENT_LOG: 0x001de000 → 0x001ee000
# NV_FTW_WORKING: 0x001df000 → 0x001ef000
# NV_FTW_WORKING data: 0x001e0000 → 0x001f0000
```

**2.2 编译固件**

```bash
cd ~/Graduation-project/rpi5-uefi
./build.sh --debug 1
```

编译成功后，固件位于：
```
rpi5-uefi/RPI_EFI.fd  (2MB，带 Secure Boot 支持)
```

**步骤3**：烧录 SD 卡

```bash
# 挂载 SD 卡（假设 /dev/sdc1）
sudo mount /dev/sdc1 /mnt/sdcard

# 复制新固件
sudo cp Build/RPi5/DEBUG_GCC5/FV/RPI_EFI.fd /mnt/sdcard/

# 同步并卸载
sudo sync
sudo umount /mnt/sdcard
```

### 7.2 UEFI Setup 配置 Secure Boot

1. **插入 SD 卡，启动 RPi5**
2. **按 ESC 进入 UEFI Setup**
3. **进入 Device Manager → Secure Boot Configuration**
4. **设置 Secure Boot Enable = TRUE**
5. **设置 Secure Boot Mode = Custom**（可选）
6. **进入 PK/KEK/db Options**
   - **KEK Options** → Add PK/KEK → 选择 `KEKKey.der`
   - **db Options** → Add db → 选择 `SigningKey.der`
7. **保存并退出**（按 F10）

### 7.3 测试验证

**测试1**：正常签名的 EFI
```bash
# 用私钥签名
python3 SignTool.py --input SerialRecv.efi \
                    --output SerialRecv.signed.efi \
                    --key keys/SigningKey.pem \
                    --cert keys/SigningKey.pub

# 复制到 SD 卡
cp SerialRecv.signed.efi /media/sdcard/EFI/BOOT/

# 启动 RPi5 → 应正常执行
```

**测试2**：未签名的 EFI
```bash
# 直接复制未签名版本
cp SerialRecv.efi /media/sdcard/EFI/BOOT/

# 启动 RPi5 → 应显示 VIOLATION 并停止
```

---

## 8. 防御效果验证

| 测试 | 条件 | 预期结果 | 实际结果 |
|------|------|---------|---------|
| 1 | 签名 EFI + Secure Boot ON | 正常执行 ✅ | |
| 2 | 未签名 EFI + Secure Boot ON | 拦截 ❌ | |
| 3 | 签名 EFI + Secure Boot OFF | 正常执行 ✅ | |
| 4 | 未签名 EFI + Secure Boot OFF | 正常执行 ⚠️ | |
| 5 | 篡改签名 EFI + Secure Boot ON | 拦截 ❌ | |

---

## 9. 完整演示流程（答辩用）

### Step 1: 介绍背景（2分钟）
- 展示 LoJax/BadUSB 攻击案例
- 解释为什么需要 Secure Boot

### Step 2: 展示当前固件（1分钟）
- 显示当前 SECURE_BOOT_ENABLE = FALSE
- 解释无签名验证的风险

### Step 3: 演示攻击（2分钟）
- 运行 SerialRecvMalicious.efi
- 展示假数据和警告信息

### Step 4: 启用 Secure Boot（2分钟）
- 修改 SECURE_BOOT_ENABLE = TRUE
- 重新编译固件
- 烧录 SD 卡

### Step 5: 配置 UEFI Setup（2分钟）
- 导入 KEKKey.der 和 SigningKey.der
- 启用 Secure Boot 模式

### Step 6: 演示防御效果（2分钟）
- 运行签名的 SerialRecv.efi → 正常
- 运行未签名的 SerialRecvMalicious.efi → 拦截

### Step 7: 总结（1分钟）
- 签名验证的优势
- 密钥管理的重要性
- 未来改进方向

---

## 10. 局限性与未来改进

### 10.1 当前方案局限

1. **密钥管理复杂**：需要手动导入密钥到 UEFI
2. **调试困难**：启用 SB 后无签名镜像无法加载
3. **PK 密钥丢失风险**：一旦锁定，无法修改安全变量
4. **运行时攻击**：SB 只保护启动过程，不保护运行时

### 10.2 未来改进方向

1. **TPM 集成**：使用 TPM 2.0 存储密钥，提供硬件级保护
2. **自动化签名**：CI/CD 流程中自动签名
3. **在线验证**：结合网络验证，实现远程 attestation
4. **硬件防护**：RPi5 可能的 TPM/Crypto elemento 模块

---

## 参考资料

- [UEFI Secure Boot Specification](https://uefi.org/specs/UEFI/2.10/27_Secure_Boot.html)
- [Authenticode Specification](https://learn.microsoft.com/en-us/windows-hardware/drivers/install/authenticode)
- [worproject/rpi5-uefi](https://github.com/worproject/rpi5-uefi)
- [edk2 SecurityPkg](https://github.com/tianocore/edk2/tree/master/SecurityPkg)
- [LoJax Attack Analysis](https://www.welivesecurity.com/2018/09/27/lojax-first-uefi-rootkit-found-wild/)
- [sbsigntool](https://github.com/joseph老/sbsigntool)

---

## 更新日志

| 日期 | 更新内容 |
|------|---------|
| 2026-04-03 | 初始版本，完成大部分内容 |
| | - 安全启动原理 |
| | - RPi5 配置说明 |
| | - 攻击演示设计 |
| | - 密钥工具实现 |
| | - 演示流程 |
