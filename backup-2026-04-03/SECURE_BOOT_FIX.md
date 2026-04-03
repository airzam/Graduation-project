# Secure Boot Configuration UI 问题修复记录

## 问题分析

### 根本原因
RPi5 UEFI 固件使用 **FVMAIN_COMPACT**（精简版），而 **SecureBootConfigDxe** 在 **FVMAIN**（完整版）中。
FVMAIN 使用 FV_IMAGE 格式封装进 FVMAIN_COMPACT，但 FVMAIN（11MB）太大，即使用 FV_IMAGE 压缩也无法fit进原来的 1.8MB 空间。

### 文件大小分析
| 文件 | 大小 | 说明 |
|------|------|------|
| FVMAIN.Fv | 11,433,280 bytes (0xae7540) | 包含 SecureBootConfigDxe |
| FVMAIN_COMPACT.Fv | 1,835,008 bytes (0x1c0000) | 原来的精简版（只有 1.8MB） |

## 最终解决方案

### 方案：增大 FD 大小 + 直接添加 SecureBootConfigDxe 到 FVMAIN_COMPACT

修改 `RPi5.fdf`：

1. **增大 FD 总大小**（行29）：
   - `Size = 0x00200000` → `Size = 0x00c800000`

2. **增大 FVMAIN_COMPACT 区域**（行60）：
   - `0x001c0000` → `0x00c00000`（12MB）

3. **调整变量存储区域偏移**：
   - NV_VARIABLE_STORE: `0x001e0000` → `0x00c20000`
   - NV_EVENT_LOG: `0x001ee000` → `0x00c40000`
   - NV_FTW_WORKING: `0x001ef000` → `0x00c41000`
   - NV_FTW_WORKING_DATA: `0x001f0000` → `0x00c42000`

4. **直接在 FVMAIN_COMPACT 中添加 SecureBootConfigDxe**（行357）：
   ```fdf
   INF ArmPlatformPkg/PrePi/PeiUniCore.inf
   INF SecurityPkg/VariableAuthenticated/SecureBootConfigDxe/SecureBootConfigDxe.inf
   FILE FV_IMAGE = 9E21FD93-9C72-4c15-8C4B-E77F1DB2D792 {
   ```

### 修改时间
2026-04-03

### 备份
原始文件已备份到 `backup-2026-04-03/` 目录

## 验证结果

```bash
# 新的固件大小
RPI_EFI.fd: 12,918,784 bytes (12.3MB)

# SecureBootConfigDxe 存在于固件中
strings RPI_EFI.fd | grep -i secureboot
# 输出: SECUREBOOT_CONFIGURATION

# GUID 验证
hexdump -C RPI_EFI.fd | grep -i "f0e6"
# 输出: 001f0e60 ... f0e6... (倒序存储)
```

## 构建输出

```
FV Space Information
FVMAIN [99%Full] 9954624 (0x97e540) total, 9954584 (0x97e518) used, 40 (0x28) free
FVMAIN_COMPACT [17%Full] 12582912 (0xc00000) total, 2242552 (0x2237f8) used, 10340360 (0x9dc808) free
```

FVMAIN_COMPACT 现在使用了 17%（2.2MB），包含 SecureBootConfigDxe。
