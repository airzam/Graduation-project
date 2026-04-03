# HostTools - UEFI Secure Boot Key and Signing Tools

本目录包含用于 UEFI Secure Boot 的密钥生成和签名工具。

## 工具列表

| 工具 | 功能 |
|------|------|
| GenKeys.py | 生成 RSA-2048/4096 密钥对（PK/KEK/Signing） |
| SignTool.py | 对 EFI 镜像进行签名 |
| VerifyTool.py | 验证签名（可选） |

## 快速开始

### 1. 安装依赖

```bash
# Python cryptography 库
pip3 install cryptography

# sbsigntool（用于真正的 UEFI 签名，推荐）
sudo apt install sbsigntool

# OpenSSL（通常已预装）
openssl version
```

### 2. 生成密钥

```bash
cd HostTools
python3 GenKeys.py --key-dir ./keys --key-size 2048
```

这会生成以下文件：
- `keys/PlatformKey.pem` - 平台密钥私钥（最高权限，**必须备份**）
- `keys/PlatformKey.pub` - 平台密钥公钥
- `keys/KEKKey.pem` - KEK 私钥
- `keys/KEKKey.pub` - KEK 公钥
- `keys/SigningKey.pem` - 签名私钥（用于签名 .efi）
- `keys/SigningKey.pub` - 签名公钥（需导入 UEFI db）
- `*.der` 文件 - DER 格式，用于 UEFI 固件直接导入

### 3. 签名 EFI 镜像

```bash
python3 SignTool.py \
    --input MyApp.efi \
    --output MyApp.signed.efi \
    --key keys/SigningKey.pem \
    --cert keys/SigningKey.pub
```

### 4. 验证签名

```bash
python3 SignTool.py --verify \
    --input MyApp.signed.efi \
    --cert keys/SigningKey.pub
```

### 5. 计算 PE Hash（用于 UEFI db 白名单）

如果你不想使用签名，而想直接将哈希加入白名单：

```bash
python3 SignTool.py --hash --input MyApp.efi
```

输出示例：
```
SHA-256: a1b2c3d4e5f6...
For UEFI db entry, format as:
  SHA256:A1B2C3D4E5F6...
```

## 密钥管理说明

### 密钥用途

```
PK (Platform Key)
├── 最高权限
├── 用于签署 KEK
├── 用于启用/禁用 Secure Boot
└── 丢失后无法修改安全变量

KEK (Key Exchange Keys)
├── 用于签署 db/dbx 更新
└── 建议至少保留两个（主密钥 + 备用密钥）

Signing Key
├── 用于签名你的 .efi 应用程序
└── 公钥需导入 UEFI db 白名单
```

### 安全警告

**重要**：
1. **不要**将 .pem 私钥文件提交到 git
2. **不要**与他人分享私钥
3. 将私钥**备份**到安全位置（加密 USB、密码管理器等）
4. 如果私钥泄露，攻击者可以签名恶意 EFI 镜像

### 备份建议

```bash
# 加密备份所有密钥
tar czf keys-backup.tar.gz keys/
gpg -c keys-backup.tar.gz  # 输入密码加密
# 将加密后的文件保存到安全位置
```

## RPi5 UEFI Secure Boot 配置流程

### Phase 1: 准备

1. 生成密钥对
2. 将 `SigningKey.der` 复制到 SD 卡

### Phase 2: 固件配置

1. 插入 SD 卡，启动 RPi5
2. 按 `ESC` 进入 UEFI Setup
3. 进入 `Device Manager` → `Secure Boot Configuration`
4. 设置 `Secure Boot Enable` = TRUE
5. 进入 `PK/KEK/db Options`
6. 按顺序导入：
   - `KEKKey.der` → Key Exchange Key (KEK)
   - `SigningKey.der` → Signature Database (db)
7. 保存并退出

### Phase 3: 测试

1. 复制**未签名**的 .efi → 应该被拦截
2. 复制**已签名**的 .efi → 应该正常运行

## 目录结构

```
HostTools/
├── README.md           # 本文件
├── GenKeys.py          # 密钥生成工具
├── SignTool.py         # 签名工具
└── VerifyTool.py       # 验证工具（TODO）
```

## 故障排除

### "sbsign not found"

```bash
sudo apt install sbsigntool
```

### "cryptography module not found"

```bash
pip3 install cryptography
```

### UEFI 仍然不执行签名的 .efi

1. 确认公钥已正确导入 db
2. 检查时间戳是否正确
3. 确认固件日期/时间设置正确
4. 尝试清除 Secure Boot 并重新配置

## 参考资料

- [UEFI Secure Boot Specification](https://uefi.org/specs/UEFI/2.10/27_Secure_Boot.html)
- [Authenticode Specification](https://learn.microsoft.com/en-us/windows-hardware/drivers/install/authenticode)
- [sbsigntool GitHub](https://github.com/joseph老/sbsigntool)
