# 博客07计划：UEFI安全启动与数字签名实战

## Context

用户毕业设计重点方向：**安全启动模块 + 数字签名验证**

### 约束条件
- ✅ 能修改 RPi5 UEFI 固件（RPI_EFI.fd）
- ❌ 别人无法修改固件（需要物理访问+签名）
- ⚠️ 威胁来源：攻击者通过网络修改 EFI 程序（.efi 文件）

### 防御目标
防止攻击者通过以下路径注入恶意代码：
```
攻击者入侵 PC/GitHub → 篡改 .efi 文件 → 用户复制到 SD 卡 → RPi 启动执行恶意代码
```

---

## 一、真实攻击案例（毕设核心亮点）

| 攻击名称 | 时间 | 目标 | 漏洞利用 | 影响 |
|---------|------|------|---------|------|
| **LoJax** | 2018 | 东欧政府机构 | UEFI固件无签名验证 | BIOS rootkit持久化，重装系统无效 |
| **Mebromi** | 2011 | Windows PC | BIOS无签名 | 首个BIOS rootkit，关闭杀软 |
| **RPi僵尸网络** | 2019 | 公网RPi设备 | SSH弱口令+无代码签名 | 组建挖矿僵尸网络 |
| **BadUSB** | 2014 | 插入未知USB设备 | 固定介质无签名验证 | 键盘模拟攻击，代码注入 |

### 攻击链示例（你的场景）

```
攻击者视角（通过网络入侵）：

1. 攻击者入侵你的 PC 或 GitHub 仓库
        ↓
2. 篡改 MyGuiFrame.efi，植入恶意代码
        ↓
3. 你"正常"下载了这个被篡改的 .efi
        ↓
4. 复制到 SD 卡
        ↓
5. RPi 启动，加载被篡改的 .efi
        ↓
6. 恶意代码执行 → 监听串口/偷数据/后门

无签名验证：攻击成功 ✅
有签名验证：启动时被拦截 ❌
```

---

## 二、系统架构设计

### 2.1 整体架构

```
┌─────────────────────────────────────────────────────────────┐
│                    安全启动架构                              │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  【主机端】PC（Linux/Windows）                              │
│                                                             │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐ │
│  │  密钥生成器   │    │   签名工具   │    │  密钥备份    │ │
│  │  GenKeys.py  │───▶│  SignTool.py │───▶│  .pem文件    │ │
│  └──────────────┘    └──────────────┘    └──────────────┘ │
│         │                   │                              │
│         │   签名后的.efi    │                              │
│         └────────┬─────────┘                              │
│                  ▼                                         │
│         【SD卡（目标设备）】                                │
│                                                             │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  【设备端】RPi5 UEFI固件                                    │
│                                                             │
│  SECURE_BOOT_ENABLE = TRUE                                 │
│                                                             │
│  1. Power On → Boot Guard                                   │
│  2. 加载 UEFI Firmware                                      │
│  3. 初始化 PK/KEK/db/dbx 密钥库                            │
│  4. 加载 .efi 前验证签名：                                  │
│     ├── 计算 Hash (SHA-256)                                 │
│     ├── 提取 PKCS#7 签名                                    │
│     ├── 与 db/dbx 比对                                      │
│     └── 通过/拒绝                                            │
│  5. 通过则执行，拒绝则停机                                   │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### 2.2 密钥层级

```
密钥层级：
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

  PK (Platform Key)        ← 你的RPi5平台密钥（最高权限）
  ├── 用途：签署KEK，启用Secure Boot模式
  ├── 备份：**必须备份到安全位置**
  └── 丢失后果：无法修改任何安全变量

  KEK (Key Exchange Keys)  ← 可信密钥签署者
  ├── 用途：签署 db/dbx 的更新
  └── 数量：至少2个（签名密钥 + 备用密钥）

  db (Signature Database)  ← 白名单
  ├── 内容：可信证书或哈希列表
  └── 示例：
      - 你的证书（签署MyGuiFrame.efi）
      - 树莓派官方证书（签署bootloader）

  dbx (Forbidden Database) ← 黑名单
  ├── 内容：已吊销的证书或哈希
  └── 示例：
      - 已知的恶意软件哈希
      - 已泄露的证书
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

---

## 三、实现方案

### 3.1 固件修改（RPi5.dsc）

文件：`rpi5-uefi/edk2-platforms/Platform/RaspberryPi/RPi5/RPi5.dsc`

```makefile
# 修改前
DEFINE SECURE_BOOT_ENABLE      = FALSE

# 修改后
DEFINE SECURE_BOOT_ENABLE      = TRUE
```

### 3.2 需要启用的组件

```makefile
# Secure Boot 依赖
!if $(SECURE_BOOT_ENABLE) == TRUE
  TpmMeasurementLib|SecurityPkg/Library/DxeTpmMeasurementLib/DxeTpmMeasurementLib.inf
  AuthVariableLib|SecurityPkg/Library/AuthVariableLib/AuthVariableLib.inf
  SecureBootConfigDxe|SecurityPkg/Universal/SecureBoot/SecureBootConfigDxe/SecureBootConfigDxe.inf
  SecureBootDefaultKeysDxe|SecurityPkg/VariableAuthenticated/SecureBootDefaultKeysDxe/SecureBootDefaultKeysDxe.inf
!endif
```

### 3.3 验证策略

```makefile
!if $(SECURE_BOOT_ENABLE) == TRUE
  gEfiSecurityPkgTokenSpaceGuid.PcdOptionRomImageVerificationPolicy|0x04
  gEfiSecurityPkgTokenSpaceGuid.PcdFixedMediaImageVerificationPolicy|0x04
  gEfiSecurityPkgTokenSpaceGuid.PcdRemovableMediaImageVerificationPolicy|0x04
!endif
```

---

## 四、攻击演示设计（毕设亮点）

### 4.1 恶意 EFI 程序（无害演示版）

文件：`edk2/SerialPkg/SerialRecvMalicious/SerialRecvMalicious.c`

```c
/**
 * SerialRecvMalicious - 恶意版本演示（无害）
 *
 * 功能：
 * 1. 显示警告信息 "⚠️ ATTENTION: This system is COMPROMISED!"
 * 2. 发送假数据到串口（伪造传感器读数）
 * 3. 记录"受害"日志到 SD 卡文件
 *
 * 注意：此代码仅用于安全演示，无实际破坏性
 **/

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>

EFI_STATUS EFIAPI UefiMain(
    IN EFI_HANDLE        ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
)
{
    // 显示警告
    Print(L"\n");
    Print(L"===========================================\n");
    Print(L"  ⚠️  WARNING: SYSTEM COMPROMISED!       \n");
    Print(L"===========================================\n");
    Print(L"  Your SerialRecv.efi has been replaced  \n");
    Print(L"  by a malicious version!                \n");
    Print(L"===========================================\n");
    Print(L"\n");

    // 模拟发送假数据（通过串口）
    CHAR8 fakeData[] = "FAKE_SENSOR_DATA: Temp=9999C, Humidity=99%\r\n";
    // ... 发送逻辑（与 SerialRecv.c 相同）

    // 模拟"记录受害"
    Print(L"[MALICIOUS] Log written to /fake/path/malware.log\n");

    Print(L"\nPress any key to exit...\n");
    // 等待按键退出

    return EFI_SUCCESS;
}
```

### 4.2 演示流程

```
阶段1：无 Secure Boot（展示攻击成功）
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
1. 当前固件 SECURE_BOOT_ENABLE = FALSE
2. 编译正常的 SerialRecv.efi → 签名工具签名 → 放到 SD 卡
3. 演示：正常版本运行，显示正常数据
4. "攻击者"替换为恶意版本 SerialRecvMalicious.efi
5. 运行恶意版本 → 显示假数据 → 证明攻击成功 ⚠️

阶段2：启用 Secure Boot（展示防御）
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
1. 修改 SECURE_BOOT_ENABLE = TRUE，重新编译固件
2. 生成签名密钥对（RSA-2048）
3. 把公钥导出为证书，导入到 db
4. 用私钥给 SerialRecv.efi 签名（生成 .signed.efi）
5. 把签名后的 .efi 放到 SD 卡
6. 再次运行 → 正常执行 ✅
7. 再替换为恶意版本（无签名）
8. 运行 → 屏幕显示：
   ╔═══════════════════════════════════════╗
   ║     SECURE BOOT VIOLATION            ║
   ║                                       ║
   ║  Image: SerialRecv.efi               ║
   ║  Error: Signature verification       ║
   ║          FAILED                       ║
   ║                                       ║
   ║  System halted.                       ║
   ╚═══════════════════════════════════════╝
9. 证明攻击被阻止 ✅
```

---

## 五、主机端工具设计

### 5.1 目录结构

```
HostTools/
├── GenKeys.py           # 密钥生成工具
├── SignTool.py          # EFI 镜像签名工具
├── VerifyTool.py        # 签名验证工具
└── README.md            # 使用说明
```

### 5.2 GenKeys.py - 密钥生成

```python
#!/usr/bin/env python3
"""
GenKeys.py - 生成 RSA-2048 密钥对用于 EFI 签名

用法：
    python3 GenKeys.py [--key-dir ./keys]

生成文件：
    - PlatformKey.pem      (PK 私钥)
    - PlatformKey.pub      (PK 公钥)
    - KEKKey.pem           (KEK 私钥)
    - KEKKey.pub           (KEK 公钥)
    - SigningKey.pem       (签名私钥，用于签署 .efi)
    - SigningKey.pub       (签名公钥，需导入 db)
"""

import os
import argparse
from cryptography.hazmat.primitives.asymmetric import rsa
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.backends import default_backend

def generate_rsa_key(key_size=2048):
    """生成 RSA 密钥对"""
    private_key = rsa.generate_private_key(
        public_exponent=65537,
        key_size=key_size,
        backend=default_backend()
    )
    return private_key

def save_key_pair(private_key, public_key, base_name, key_dir):
    """保存密钥对到文件"""
    os.makedirs(key_dir, exist_ok=True)

    # 私钥
    private_pem = private_key.private_bytes(
        encoding=serialization.Encoding.PEM,
        format=serialization.PrivateFormat.TraditionalOpenSSL,
        encryption_algorithm=serialization.NoEncryption()
    )
    private_path = os.path.join(key_dir, f"{base_name}.pem")
    with open(private_path, 'wb') as f:
        f.write(private_pem)

    # 公钥
    public_pem = public_key.public_bytes(
        encoding=serialization.Encoding.PEM,
        format=serialization.PublicFormat.SubjectPublicKeyInfo
    )
    public_path = os.path.join(key_dir, f"{base_name}.pub")
    with open(public_path, 'wb') as f:
        f.write(public_pem)

    print(f"Generated: {private_path}")
    print(f"Generated: {public_path}")

def main():
    parser = argparse.ArgumentParser(description='Generate RSA keys for EFI Secure Boot')
    parser.add_argument('--key-dir', default='./keys', help='Directory to store keys')
    parser.add_argument('--key-size', type=int, default=2048, help='RSA key size (2048 or 4096)')
    args = parser.parse_args()

    print("=" * 50)
    print("EFI Secure Boot Key Generator")
    print("=" * 50)

    # 生成 Platform Key
    print("\n[1/3] Generating Platform Key (PK)...")
    pk_private = generate_rsa_key(args.key_size)
    save_key_pair(pk_private, pk_private.public_key(), "PlatformKey", args.key_dir)

    # 生成 KEK
    print("\n[2/3] Generating Key Exchange Key (KEK)...")
    kek_private = generate_rsa_key(args.key_size)
    save_key_pair(kek_private, kek_private.public_key(), "KEKKey", args.key_dir)

    # 生成签名密钥
    print("\n[3/3] Generating Signing Key...")
    sign_private = generate_rsa_key(args.key_size)
    save_key_pair(sign_private, sign_private.public_key(), "SigningKey", args.key_dir)

    print("\n" + "=" * 50)
    print("Key generation complete!")
    print("=" * 50)
    print(f"\nIMPORTANT: Backup your keys to a secure location!")
    print(f"Keys are stored in: {os.path.abspath(args.key_dir)}")
    print("\nNext steps:")
    print("1. Import SigningKey.pub into your UEFI db")
    print("2. Use SigningKey.pem to sign your .efi files")

if __name_module__ == '__main__':
    main()
```

### 5.3 SignTool.py - EFI 签名工具

```python
#!/usr/bin/env python3
"""
SignTool.py - 对 EFI 镜像进行签名（生成 PKCS#7 签名）

用法：
    python3 SignTool.py --input MyApp.efi --output MyApp.signed.efi \
                        --key SigningKey.pem --cert SigningKey.pub

依赖：
    pip3 install cryptography
    (需要 OpenSSL 命令行工具)
"""

import os
import argparse
import hashlib
import subprocess

def sign_pe_image(input_file, output_file, key_pem, cert_pem):
    """
    对 PE/COFF 镜像签名

    实际使用 sbsign 或 pesign 工具
    """
    # 方案1：使用 sbsign (Linux)
    # sbsign --key key.pem --cert cert.pem --output output.efi input.efi

    # 方案2：使用 openssl 直接计算签名，手动附加到文件
    pass

def calculate_pe_hash(pe_file):
    """
    计算 PE/COFF 镜像的 SHA-256 Hash（Authenticode）
    """
    with open(pe_file, 'rb') as f:
        data = f.read()

    # 跳过 DOS header 的 e_lfanew 字段指向的位置
    # 实际实现需要解析 PE 结构，计算合适的范围
    sha256 = hashlib.sha256()
    sha256.update(data)
    return sha256.hexdigest()

def main():
    parser = argparse.ArgumentParser(description='Sign EFI images')
    parser.add_argument('--input', '-i', required=True, help='Input .efi file')
    parser.add_argument('--output', '-o', required=True, help='Output signed .efi file')
    parser.add_argument('--key', required=True, help='Signing key (.pem)')
    parser.add_argument('--cert', required=True, help='Signing certificate (.pem)')
    parser.add_argument('--hash', action='store_true', help='Only calculate hash, do not sign')
    args = parser.parse_args()

    if not os.path.exists(args.input):
        print(f"Error: Input file not found: {args.input}")
        return 1

    if args.hash:
        # 仅计算 Hash
        h = calculate_pe_hash(args.input)
        print(f"PE Image Hash: {h}")
        print(f"Hash Algorithm: SHA-256")
    else:
        # 执行签名
        print(f"Signing {args.input}...")
        print(f"Output: {args.output}")

        # TODO: 实现完整签名逻辑
        # 1. 计算 PE Hash
        # 2. 用私钥生成 PKCS#7 签名
        # 3. 将签名附加到 EFI 镜像

        print("Signing not yet implemented - use OpenSSL manually:")
        print(f"openssl cms -sign -inkey {args.key} -signer {args.cert} \\")
        print(f"  -binary -outform DER -out {args.output}.pkcs7 \\")
        print(f"  {args.input}")

    return 0

if __name__ == '__main__':
    exit(main())
```

---

## 六、博客07大纲

```
07-UEFI安全启动与数字签名实战.md
├── 1. 研究背景：那些崩溃世界的固件漏洞
│   ├── LoJax：首个UEFI rootkit（2018）
│   ├── Mebromi：BIOS bootskit时代开启（2011）
│   ├── RPi僵尸网络：嵌入式设备的隐忧（2019）
│   └── BadUSB：即插即攻的威胁（2014）
│
├── 2. 威胁建模：你的RPi5正在被攻击吗？
│   ├── 当前SECURE_BOOT_ENABLE=FALSE的隐患
│   ├── 攻击面分析（SD卡/串口/网络）
│   └── 攻击链展示（从GitHub到RPi）
│
├── 3. UEFI安全启动原理
│   ├── PK/KEK/db/dbx密钥层级
│   ├── 签名验证流程（Hash+PKCS#7）
│   ├── DEFER_EXECUTE_ON_SECURITY_VIOLATION策略
│   └── 镜像验证策略（IMAGE_FROM_FIXED_MEDIA等）
│
├── 4. RPi5 UEFI安全框架（代码分析）
│   ├── DxeImageVerificationLib分析
│   │   ├── DxeImageVerificationHandler()
│   │   ├── HashPeImage() - PE/COFF哈希计算
│   │   ├── IsAllowedByDb() / IsForbiddenByDbx()
│   │   └── 支持算法：SHA1/SHA256/SHA384/SHA512
│   ├── AuthVariableLib分析
│   │   ├── ProcessVarWithPk() / ProcessVarWithKek()
│   │   └── 证书链验证
│   └── SECURE_BOOT_ENABLE开关解析
│
├── 5. 攻击演示：故意被攻击以展示危害
│   ├── 5.1 恶意EFI程序设计（无害演示版）
│   │   ├── SerialRecvMalicious.c 功能
│   │   └── 演示警告信息设计
│   ├── 5.2 无Secure Boot：攻击成功
│   │   └── 截图：假数据、受害日志
│   └── 5.3 启用Secure Boot：攻击被拦截
│       └── 截图：SECURE BOOT VIOLATION
│
├── 6. 密钥管理工具设计与实现
│   ├── 6.1 GenKeys.py - RSA-2048密钥对生成
│   │   ├── 依赖：cryptography库
│   │   └── 生成：PK/KEK/Signing三组密钥
│   ├── 6.2 SignTool.py - PE/COFF镜像签名
│   │   ├── 依赖：OpenSSL/sbsign
│   │   └── PKCS#7签名格式
│   └── 6.3 VerifyTool.py - 签名验证工具
│       └── 用于PC端预验证
│
├── 7. 实战：启用RPi5安全启动
│   ├── 7.1 修改RPi5.dsc
│   │   └── SECURE_BOOT_ENABLE = TRUE
│   ├── 7.2 重新编译固件
│   │   ├── build -a AARCH64 -t GCC5 -b DEBUG
│   │   └── 生成新的 RPI_EFI.fd
│   ├── 7.3 烧录SD卡
│   └── 7.4 UEFI Setup配置
│       ├── 进入Secure Boot Configuration
│       ├── 安装PK/KEK/db
│       └── 设置Custom Mode
│
├── 8. 防御效果验证
│   ├── 测试1：正常签名的efi能否启动 ✅
│   ├── 测试2：无签名efi是否被阻止 ❌
│   ├── 测试3：篡改后的efi是否被检测
│   └── 测试4：dbx吊销功能验证
│
├── 9. 完整演示流程（答辩用）
│   ├── Step 1: 介绍攻击案例（LoJax/BadUSB）
│   ├── Step 2: 展示当前固件无Secure Boot
│   ├── Step 3: 运行恶意efi，显示"受害"
│   ├── Step 4: 启用Secure Boot，重新签名
│   ├── Step 5: 再运行恶意efi，显示Violation
│   └── Step 6: 总结防御效果
│
└── 10. 总结与展望
    ├── 签名验证的优势总结
    ├── 局限性讨论（运行时攻击、密钥管理）
    └── 未来改进方向（自动化签名、TPM集成）
```

---

## 七、关键文件清单

### 7.1 固件修改

| 文件 | 修改内容 |
|------|---------|
| `rpi5-uefi/edk2-platforms/Platform/RaspberryPi/RPi5/RPi5.dsc` | `SECURE_BOOT_ENABLE = TRUE` |
| `rpi5-uefi/edk2-platforms/Platform/RaspberryPi/RPi5/RPi5.fdf` | 添加 SecureBootConfigDxe.inf |

### 7.2 新增文件

| 文件 | 用途 |
|------|------|
| `rpi5-uefi/edk2/SerialPkg/SerialRecvMalicious/` | 恶意版本演示程序 |
| `HostTools/GenKeys.py` | 密钥生成工具 |
| `HostTools/SignTool.py` | EFI签名工具 |
| `HostTools/VerifyTool.py` | 签名验证工具 |
| `blog/07-UEFI安全启动与数字签名实战.md` | 博客文章 |

---

## 八、实施步骤

### Phase 1：基础启用（1天）
- [ ] 修改 `SECURE_BOOT_ENABLE = TRUE`
- [ ] 重新编译固件
- [ ] 烧录 SD 卡
- [ ] 进入 UEFI Setup 观察 Secure Boot 配置界面

### Phase 2：攻击演示程序（1天）
- [ ] 创建 `SerialRecvMalicious.c`
- [ ] 编译恶意版本
- [ ] 设计无害的"受害"效果

### Phase 3：签名工具（2天）
- [ ] 实现 GenKeys.py
- [ ] 实现 SignTool.py
- [ ] 在 PC 上测试签名流程

### Phase 4：完整演示（1天）
- [ ] 无 Secure Boot 状态演示
- [ ] 启用 Secure Boot
- [ ] 签名 efi
- [ ] 拦截演示
- [ ] 录制答辩视频/截图

### Phase 5：文档撰写（1天）
- [ ] 完成博客07
- [ ] 更新 FOR_AI.md
- [ ] 推送到 GitHub

---

## 九、风险与注意事项

1. **PK 密钥丢失**：一旦设置 PK 并锁定，无法修改安全变量
   - **缓解**：保留私钥备份到多个安全位置

2. **调试困难**：启用 SB 后无签名镜像无法加载
   - **缓解**：保留一个带调试签名的 UEFI Shell

3. **RPi5 固件限制**：
   - Bcm57xxx 网卡驱动缺失（PXE启动不可用）
   - 但不影响本地 .efi 签名验证

---

## 十、参考资料

- [UEFI Secure Boot Specification](https://uefi.org/specs/UEFI/2.10/27_Secure_Boot.html)
- [Authenticode Specification](https://learn.microsoft.com/en-us/windows-hardware/drivers/install/authenticode)
- [worproject/rpi5-uefi](https://github.com/worproject/rpi5-uefi)
- [edk2 SecurityPkg](https://github.com/tianocore/edk2/tree/master/SecurityPkg)
