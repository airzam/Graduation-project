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
- 新增博客 `04-windows虚拟机配置SSH.md`

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
│   └── 05-*.md            # 博客文章
├── code-analysis/          # 代码分析文档
│   └── 01-MyGuiFrame详解.md
├── rpi5-uefi/              # UEFI 固件源码
│   └── edk2/
│       ├── MyAppPkg/       # 自己的应用包
│       └── SerialPkg/      # 串口通信包（SerialRecv.efi 已编译成功）
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
