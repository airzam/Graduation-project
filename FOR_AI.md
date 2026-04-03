# ⚠️ 写给 AI 看的说明

本文档是用户（airzam/黎民赞）记录给 AI 看的操作手册。
包含 AI 犯过的错误及纠正方法，**每次对话结束后必须更新本文件的对话记录**。

---

## 基本信息

- **用户**：airzam（黎民赞），测控技术与仪器专业
- **课题**：基于树莓派5的UEFI启动程序设计
- **GitHub**：https://github.com/airzam/Graduation-project
- **工程路径**：`C:\Users\67426\Desktop\毕业设计\`

## 开发环境

- AI 模型：MiniMax M2.7
- 构建平台：Linux（VMware 虚拟机 + Ubuntu）
- 交叉编译器：gcc-aarch64-linux-gnu
- 网络代理：`http://192.168.186.1:7897`（主机 Clash 局域网连接）
- **sudo 密码**：`123456`（已配置，无需每次询问）

## Git 仓库

```bash
git remote add origin git@github.com:airzam/Graduation-project.git
```

---

## 🤖 AI 协作分工

本项目由两个 AI 协作维护，**最明显的区别是运行环境不同**：

| 运行环境 | AI 端 | 职责 |
|---------|-------|------|
| **WSL**（当前会话） | **AI-A（WSL）** | 代码开发、调试、版本管理、git 推送 |
| **Windows** | **AI-B（Windows）** | Word、Excel 等大文件修改 |

### 协作规则

1. **AI-A（WSL）**
   - 在 WSL/Linux 环境（`/mnt/c/Users/67426/Desktop/毕业设计/`）工作
   - 负责代码开发、调试、版本管理
   - 通过 git 推送更新到 GitHub
   - 保持代码仓库整洁，大文件由 Git LFS 管理
   - 每次对话结束后更新 FOR_AI.md 并提交

2. **AI-B（Windows）**
   - 在 Windows 环境工作
   - 负责 Word 文档、Excel 等大文件修改
   - 修改完成后告知 AI-A 同步状态

3. **同步机制**
   - 代码相关：AI-A 通过 git 管理
   - 大文件：AI-B 在本地修改，完成后告知 AI-A 同步状态
   - 每次对话结束后，双方都应更新 FOR_AI.md 的对话记录

---

## ⚠️ AI 犯过的错误记录

### 2026-03-30

1. **README 来源混淆**
   - 错误：把 `rpi5-uefi-master/README.md` 当成自己写的项目说明
   - 实际情况：这是上游 worproject 项目的原始 README，不是用户的
   - 纠正：创建了 `毕业设计/FOR_AI.md` 和 `rpi5-uefi-master/FOR_AI.md` 作为 AI 说明
   - **教训**：遇到 README.md 要先确认来源，是上游的还是用户自己的

2. **工程路径混淆**
   - 错误：以为工程在 OneDrive
   - 实际情况：已移动到桌面 `C:\Users\67426\Desktop\毕业设计\`
   - **教训**：每次操作前确认工程实际路径

### 2026-03-31（AI-A WSL）

1. **git reset --hard 导致文件丢失**
   - 错误：执行 `git reset --hard origin/main` 后，本地 staged（未 commit）的文件从 git 索引和本地文件系统同时被删除
   - 损失：论文文档（开题报告、文献综述、任务书、选题汇总表）、API KEY.txt、github令牌.txt 等
   - 恢复：用户从 Windows 回收站/备份手动恢复了文件
   - **教训**：
     - `git reset --hard` 会同时删除 git 索引和本地文件，危险！
     - 未 commit 的文件不受 git 保护，丢失无法通过 git 恢复
     - commit 前务必确认暂存区内容，或使用 `git stash` 而非 `git reset`
   - **预防**：对于未 commit 的重要文件，先备份再操作

2. **推送包含超大文件和密钥**
   - 错误：尝试推送超过 GitHub 100MB 限制的文件（rpi5-uefi-master.zip 105MB、UEFI编程实践_.pdf 54MB）
   - 错误：推送了 github令牌.txt（触发 GitHub secret scanning）
   - 纠正：从 git 历史中移除这些文件，更新 .gitignore 排除 *.pdf 和超大 zip
   - **教训**：提交前检查文件大小，超大文件（>50MB）不应进 git

---

## 对话记录

### 2026-03-28

- 创建 README.md
- 撰写博客《在 Windows 上搭建开发环境：虚拟机 + Ubuntu + Claude Code》
- 创建 blog/README.md 博客编写规范

### 2026-03-29

- 配置 MiniMax M2.7 模型
- 推送代码至 GitHub
- 工程移至 OneDrive
- 精简博客内容，添加 VMware Tools、VSCode、MiniMax 配置
- 创建 .gitignore，初始化 Git 仓库

### 2026-03-30

- 移动工程从 OneDrive 到桌面
- 创建 `毕业设计/FOR_AI.md`（项目主说明）
- 封存 `rpi5-uefi-master/` 为 `rpi5-uefi-master.zip`（本地上传，不进 git）
- 重写本文件为 AI 操作历史记录

### 2026-03-30（AI-A WSL）

- 克隆项目到 ~/Graduation-project
- 添加 AI 协作分工说明（AI-A WSL vs AI-B Windows）
- 明确分工：AI-A 负责代码和 git，AI-B 负责大文件
- 生成 SSH key（ed25519），提供公钥供用户添加到 GitHub
- 配置 git 用户信息（airzam）
- 切换 remote 为 SSH 方式并成功推送到 GitHub
- 新增博客 `02-git多用户协作与SSH配置.md`
  - 第一部分：SSH key 配置、多 AI 协作分工
  - 第二部分（新增）：Git 常用命令详解（12 个分类）、分支策略、冲突处理、提交规范

### 2026-03-31（AI-A WSL）

- 配置 VMware 虚拟机代理上网
- 克隆 worproject/rpi5-uefi 源码
- 安装编译依赖并修复子模块
- 编译成功，生成 RPI_EFI.fd
- SD 卡烧录固件

### 2026-04-01（Linux AI）

- **添加 USB 鼠标驱动（UsbMouseDxe）**
  - 修改 `Platform/RaspberryPi/RPi5/RPi5.dsc`
  - 修改 `Platform/RaspberryPi/RPi5/RPi5.fdf`
  - 重新编译固件，替换 SD 卡
- 更新第三篇博客，添加鼠标驱动说明
  - 主机代理地址：`192.168.186.1:7897`（Clash 局域网模式）
  - 代理写入 `~/.bashrc`，新终端自动生效
  - NAT 模式下主机代理不能用 `127.0.0.1`，要用 VMware NAT 网关 IP `192.168.186.1`
- 克隆 worproject/rpi5-uefi 源码到项目目录
- 安装编译依赖：gcc-aarch64-linux-gnu、iasl、python3-pyelfutils、uuid-dev 等
- 修复子模块缺失问题（Brotli、MipiSysT、MbedTLS、Cmocka 等）
- 编译成功，生成 `RPI_EFI.fd`（2MB）和 `config.txt`
- SD 卡分区（FAT32）并烧录固件
- 新增博客 `03-编译rpi5-uefi并烧录SD卡.md`
- 配置 Git LFS 管理大文件（.docx/.xlsx/.zip/.fd 等）
- 更新 .gitignore 和同步规则说明
- **【错误】** 执行 `git reset --hard` 导致本地文件丢失
  - 原因：未 commit 的 staged 文件被同时从 git 索引和本地文件系统删除
  - 教训：重要文件先 commit 再操作；使用 `git stash` 而非 `git reset --hard`
- 同步论文文档到 GitHub（用户恢复文件后）
- 推送成功

### 2026-04-01 晚（Linux AI）

- **配置 VMware Ubuntu SSH 服务**
  - 安装 openssh-server
  - 启动 sshd 服务并设置开机自启
  - IP 地址：192.168.186.135
- 新增博客 `06-windows虚拟机配置SSH.md`

### 2026-04-02（Linux AI）

- **用户从 Windows 上传了 edk2 工程到 home 目录**
- 对比两个 edk2：
  - 毕设 edk2（worproject/rpi5-uefi）：RPi5 固件编译用
  - 上传 edk2（tianocore/edk2）：UEFI 应用开发用（含 RobinPkg、StdLib、HelloWorldPkg）
- **实现 MyGuiFrame 从零编译**
  - 创建 `MyAppPkg` 包
  - 从上传 edk2 复制 StdLib、HelloWorldPkg、StackCheckLib 到毕设 edk2
  - 配置 `MyAppPkg.dsc` 和 `MyGuiFrame.inf`
  - 编写 `MyGuiFrame.c`（Hello World 版）
  - **修复 7 个编译问题**：
    1. 库依赖链缺失 → 逐步添加所有依赖库
    2. `__stack_chk_guard` 未定义 → 添加 StackCheckLib
    3. `unsupported relocation` → 添加 `-fno-stack-protector`
    4. `WaitKey()` 未定义 → 使用 `gST->ConIn->ReadKeyStroke()`
    5. `gST`/`gBS` 未声明 → 包含 `UefiBootServicesTableLib.h`
    6. `PLATFORM_VERSION` 缺失 → 在 DSC 添加版本定义
    7. LTO 与 GCC 13 不兼容 → 禁用栈保护
  - 成功编译 MyGuiFrame.efi (16KB)
  - 复制到 SD 卡
  - 新增博客 `04-实现MyGuiFrame应用.md`（含完整流程和问题汇总）
- **双 edk2 分工方案确认**：
  - 毕设 edk2：编译 RPi5 固件 + 自己写的 EFI 程序
  - 上传 edk2：参考（提取必要的包）
- 推送所有更改到 GitHub

### 2026-04-02 续（Linux AI）

- **创建代码分析文档**
  - 新建 `code-analysis/` 文件夹
  - 创建 `01-MyGuiFrame详解.md`，按逻辑顺序解析 MyGuiFrame 实现
  - 内容包括：程序入口、协议初始化、图形模式切换、Blt操作、GUI框架、事件循环
  - 包含协议汇总表、全局变量表、关键代码片段、文件依赖关系

### 2026-04-02 再续（Linux AI）

- **尝试网络启动方案（失败）**
  - 分析 RPi5 UEFI 源码：NetworkPkg/PXE 已包含，但缺 Bcm57xxx 网卡驱动
  - 结论：Boot Manager 里没有 PXE 选项，因为 UEFI 阶段网卡驱动缺失
- **改用串口通信方案**
  - 创建 `SerialPkg` 包（串口通信功能预留）
  - 创建 `SerialRecv.c`（串口回环测试程序）
  - **构建问题**：SerialPkg 构建在 "Processing meta-data" 阶段失败
  - 新增博客 `05-树莓派串口通信.md`

### 2026-04-02 续-2（Linux AI）

- **修复 SerialPkg 构建问题**
  - 问题：复制 MyAppPkg 后文件未正确重命名
  - 解决：将 MyAppPkg.dec/dsc → SerialPkg.dec/dsc，MyGuiFrame.inf/c → SerialRecv.inf/c
  - 更新所有 DEC/DSC/INF 文件内容中的包名和路径
  - **SerialPkg.dsc 缺少 GCC5_AARCH64_PREFIX**：`export GCC5_AARCH64_PREFIX=aarch64-linux-gnu-`
  - SerialRecv.efi (16KB) 编译成功
- **SD 卡状态**
  - SD 卡已断开连接（设备 /dev/sdc1 不存在）
  - 用户需重新插卡后复制 SerialRecv.efi
- 推送 SerialPkg 修复到 GitHub

### 2026-04-02 续-3（Linux AI）

- **重大失误：串口代码用错了协议**
  - 错误：SerialRecv.c 使用 `gST->ConIn->ReadKeyStroke()` 和 `gST->ConOut->OutputString()`
  - 这是 **Console 协议**（键盘/屏幕），不是 GPIO 串口
  - 后果：PC 发送数据到 GPIO → RPi 能收到并显示，但无法通过 GPIO 回传
  - 原因：不懂 Console 协议 vs Serial I/O 协议的区别
  - **教训**：UEFI 有两套输入输出系统
    - Console 协议：键盘/HDMI（`gST->ConIn/ConOut`）
    - Serial I/O 协议：GPIO 串口（需 `LocateProtocol` 获取）
  - **修复**：重写 SerialRecv.c，使用 `EFI_SERIAL_IO_PROTOCOL` 的 `Read()`/`Write()`
  - 自定义协议结构体（避免 gEfiSerialIoProtocolGuid 链接问题）
  - SerialRecv.efi 重新编译成功

### 2026-04-02 续-4（Linux AI）

- **GPIO 14/15 串口不工作：根本原因**
  - 代码分析 `Rhpx.asl` 发现 GPIO 14/15 UART 复用被注释禁用
  - 原因：DMA 冲突（"dmap conflict"）
  - **RPi5 UEFI 固件不支持 GPIO 14/15 串口**
- **RPi5 有独立的专用串口连接器**
  - README.md 确认：UART 🟢 工作，PL011 在 dedicated connector
  - 这是 3-pin 独立插针（非 40-pin GPIO），位置在板子其他区域
  - 用户已找到该专用串口
- **SerialRecv.v2 功能**
  - 使用 Serial I/O 协议
  - 枚举所有 Serial I/O 端口
  - 每 5 秒自动发送测试数据
  - 支持 1024x600 分辨率
- **博客 05 已全面更新**
  - 修正：GPIO 14/15 不工作，应用专用串口
  - 添加：固件禁用 GPIO 串口的说明
  - 更新：硬件连接图和常见问题

### 2026-04-03（Linux AI）

- **毕业设计重点：安全启动模块**
  - 用户确认需求：自己能改固件，别人改不了，但别人可能通过网络改 EFI 程序
  - 这符合 LoJax/BadUSB 等真实攻击案例的防御场景
  - **攻击链**：GitHub被篡改 → 下载恶意.efi → 复制到SD卡 → RPi启动执行恶意代码
  - **防御方案**：启用 UEFI Secure Boot，签名验证 .efi
- **分析了4种实现方案**：
  - 方案 A：每个 EFI 应用自验签名（难度★★☆）
  - 方案 B：独立验证器 + 白名单（难度★★☆）
  - 方案 C：加密通信 + Hash 校验（难度★★★☆）
  - 方案 D：公钥内嵌 + 应用自验（难度★★★★☆）
  - **推荐**：直接启用 SECURE_BOOT_ENABLE = TRUE（标准 UEFI 方案）
- **攻击演示设计**：
  - SerialRecvMalicious.c（无害恶意版本）：显示警告、发送假数据、记录假日志
  - 阶段1：无 Secure Boot → 攻击成功（显示假数据）
  - 阶段2：启用 Secure Boot → 恶意版本被拦截（显示 Violation）
- **保存计划文档**：`docs/PLAN-07-安全启动设计.md`

**今日已完成**：
- [x] 修改 SECURE_BOOT_ENABLE = TRUE（RPi5.dsc）
- [x] 创建 SerialRecvMalicious.c + .inf
- [x] 实现 HostTools（GenKeys.py, SignTool.py, README.md）
- [x] 完成博客 07（完整版）
- [x] 编译 SerialRecvMalicious.efi
- [x] 生成签名密钥（KEKKey.der, SigningKey.der）
- [x] 签名 SerialRecv.efi
- [x] 修复 FDF 文件编译问题
- [x] 重新编译带 Secure Boot 的固件
- [x] **修复 SecureBootConfigDxe 未出现在固件中的问题**
  - 直接在 FVMAIN_COMPACT 中添加 SecureBootConfigDxe.inf
  - 增大 FD 大小到 12.5MB，FVMAIN_COMPACT 到 12MB
  - 验证：RPI_EFI.fd 包含 SecureBootConfigDxe
- [ ] SD 卡烧录新固件并测试 Secure Boot Configuration UI
- [ ] 答辩演示

---

## 遇到的问题与解决方案

### 问题 1：SerialRecvMalicious.c 编译错误

**错误**：
1. `error: expected '=', ',', ';', 'asm' or '__attribute__' before '*' token` - 错误的宏定义
2. `error: passing argument 2 of 'Gop->Blt' from incompatible pointer type` - UINT32* vs EFI_GRAPHICS_OUTPUT_BLT_PIXEL*
3. `error: implicit declaration of function 'AsciiSPrint'` - 未声明函数

**解决方案**：
1. 删除错误的宏定义（#define MALICIOUS_REDEFI_STATUS...）
2. 修正 BltBuffer 类型：UINT32* → EFI_GRAPHICS_OUTPUT_BLT_PIXEL*
3. 使用固定字符串替代 AsciiSPrint

### 问题 2：SignTool.py 语法错误

**错误**：`IndentationError: unindent does not match any outer indentation level`

**解决方案**：修正缩进（` EFI_GUID_SECURITY_DATABASE` → `EFI_GUID_SECURITY_DATABASE`）

### 问题 3：sbsign 无法加载证书

**错误**：`Can't read certificate from file 'keys/SigningKey.pub'`

**原因**：GenKeys.py 生成的 .pub 文件是 PEM 格式的公钥，但不是 X.509 证书格式

**解决方案**：用 OpenSSL 从私钥生成自签名证书：
```bash
openssl req -new -x509 -key keys/SigningKey.pem -out keys/SigningKey.crt
```

### 问题 4：RPi5 固件编译 FV 大小不足

**错误**：`GenFv: ERROR 3000: Invalid - the required fv image size 0x1bc1e8 exceeds the set fv image size 0x1b0000`

**原因**：启用 Secure Boot 后固件变大（增加约 48KB），原 FVMAIN_COMPACT 大小 0x1b0000 不够

**解决方案**：
1. 修改 `RPi5.fdf`：FVMAIN_COMPACT 大小从 0x001b0000 → 0x001c0000
2. 调整变量存储区域偏移：
   - NV_VARIABLE_STORE: 0x001d0000 → 0x001e0000
   - NV_EVENT_LOG: 0x001de000 → 0x001ee000
   - NV_FTW_WORKING: 0x001df000 → 0x001ef000
   - NV_FTW_WORKING data: 0x001e0000 → 0x001f0000
3. 增加 FD 总大小：0x001f0000 → 0x00200000

**修改的文件**：
- `edk2-platforms/Platform/RaspberryPi/RPi5/RPi5.fdf`

### 问题 5：SecureBootConfigDxe 未出现在最终固件中

**问题分析**：
- SecureBootConfigDxe 在 RPi5.dsc 中被条件编译，链接到 FVMAIN（11MB）
- FVMAIN_COMPACT 使用 FV_IMAGE 格式包含 FVMAIN
- 即便增大 FVMAIN_COMPACT 到 12MB，FVMAIN 仍然太大无法fit（只使用了 14%）
- 根本原因：FV_IMAGE section 无法容纳 FVMAIN

**最终解决方案**：
1. 继续增大 FD 大小到 12.5MB（0x00c80000）
2. 增大 FVMAIN_COMPACT 到 12MB（0x00c00000）
3. **关键修复**：直接在 FVMAIN_COMPACT 中添加 SecureBootConfigDxe.inf：
   ```fdf
   INF ArmPlatformPkg/PrePi/PeiUniCore.inf
   INF SecurityPkg/VariableAuthenticated/SecureBootConfigDxe/SecureBootConfigDxe.inf
   FILE FV_IMAGE = 9E21FD93-9C72-4c15-8C4B-E77F1DB2D792 {
   ```

**验证结果**：
```bash
# RPI_EFI.fd: 12,918,784 bytes (12.3MB)
strings RPI_EFI.fd | grep -i secureboot
# 输出: SECUREBOOT_CONFIGURATION

# GUID F0E6A44F-7195-41C3-AC64-54F202CD0A21 存在于固件中
```

**今日完成**：
- [x] 修复 SecureBootConfigDxe 未出现在固件中的问题
- [x] 重新编译固件，RPI_EFI.fd 包含 SecureBootConfigDxe
- [ ] SD 卡烧录新固件并测试

---

**待续**：
- 配置 UEFI Setup（导入密钥）
- 录制答辩演示视频

---

## 硬件使用经验

---

## 目录结构

```
毕业设计/
├── FOR_AI.md              # AI 操作历史记录（本文档）
├── README.md               # 项目总览
├── blog/                   # 技术博客
│   ├── README.md          # 博客写作规范
│   ├── 01-*.md            # 博客文章
│   ├── 02-*.md            # 博客文章
│   ├── 03-*.md            # 博客文章
│   ├── 04-*.md            # 博客文章
│   ├── 05-*.md            # 博客文章
│   ├── 06-*.md            # 博客文章
│   └── 07-*.md            # 博客文章（安全启动，已完成）
├── docs/                    # 设计文档
│   └── PLAN-07-安全启动设计.md  # 安全启动毕设计划
├── code-analysis/          # 代码分析文档
│   └── 01-MyGuiFrame详解.md
├── rpi5-uefi/              # UEFI 固件源码
│   └── edk2/
│       ├── MyAppPkg/       # 自己的应用包
│       ├── SerialPkg/      # 串口通信包（SerialRecv.efi 已编译成功）
│       │   └── SerialRecvMalicious/  # 恶意版本演示程序（安全演示用）
│       └── HostTools/      # 密钥生成和签名工具
├── RPi5_UEFI_Release_v0.3/ # 编译好的固件（本地）
└── *.docx / *.xlsx        # 论文文档
```

## 同步规则

| 文件类型 | 同步方式 |
|---------|---------|
| 代码、.md | GitHub（git 追踪） |
| Word、Excel、固件 | GitHub（**Git LFS** 追踪） |
| 超大文件（*.pdf、>100MB） | **本地保留**，不进 git |

### Git LFS 配置

已配置 Git LFS 追踪以下文件类型：
- `*.docx` `*.xlsx` `*.xls` `*.doc`（Office 文档）
- `*.fd` `*.efi`（固件文件）

**注意**：*.zip 和 *.rar 不由 Git LFS 追踪，超大文件本地保留即可。

---

## 注意事项

1. 工程在桌面，不在 OneDrive
2. 遇到 README.md 先确认来源（上游 vs 用户自己的）
3. **重要**：`git reset --hard` 会同时删除 git 索引和本地文件，未 commit 的文件会丢失！
4. 超大文件（*.pdf、>100MB）不进 git
5. 每次对话结束后更新对话记录

---

## 硬件使用经验

### 树莓派 5 USB 问题

- **现象**：上电前接了键盘和屏幕的供电，键盘无法使用；不接屏幕供电时键盘正常
- **原因**：可能是电源功率不足，屏幕和键盘争抢供电
- **建议**：使用独立电源供电（5V 3A 以上），或先只接键盘再根据需要接屏幕
