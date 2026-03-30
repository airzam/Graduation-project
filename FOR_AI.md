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

本项目由两个 AI 协作维护：

| 职责 | AI 端 |
|------|-------|
| 代码更新、版本管理、git 推送 | **Linux AI（本 AI）** |
| 大文件修改（Word、Excel 等） | **Windows AI** |

### 协作规则

1. **Linux AI（我）**
   - 负责代码开发、调试、版本管理
   - 通过 git 推送更新到 GitHub
   - 保持代码仓库整洁，大文件不上传
   - 每次对话结束后更新 FOR_AI.md 并提交

2. **Windows AI**
   - 负责 Word 文档、Excel 等大文件修改
   - 在 Windows 本地工作，不涉及 git

3. **同步机制**
   - 代码相关：Linux AI 通过 git 管理
   - 大文件：Windows AI 在本地修改，完成后告知 Linux AI 同步状态
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

### 2026-03-30（Linux AI）

- 克隆项目到 ~/Graduation-project
- 添加 AI 协作分工说明（Linux AI vs Windows AI）
- 明确分工：Linux AI 负责代码和 git，Windows AI 负责大文件
- 生成 SSH key（ed25519），提供公钥供用户添加到 GitHub
- 配置 git 用户信息（airzam）
- 切换 remote 为 SSH 方式并成功推送到 GitHub
- 新增博客 `02-git多用户协作与SSH配置.md`
  - 第一部分：SSH key 配置、多 AI 协作分工
  - 第二部分（新增）：Git 常用命令详解（12 个分类）、分支策略、冲突处理、提交规范

### 2026-03-31（Linux AI）

- 配置 VMware 虚拟机代理上网
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
- 告知 Windows AI：提交大文件前需运行 `git lfs install`
- UEFI 源码已 fork 到 GitHub：https://github.com/airzam/rpi5-uefi
  - 手动修复 subhook（Gitee 镜像）：`git clone https://gitee.com/fanxingkong/subhook.git subhook`

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
│   └── 03-*.md            # 博客文章
├── rpi5-uefi/              # UEFI 固件源码（fork 自 worproject/rpi5-uefi）
│   └── GitHub：https://github.com/airzam/rpi5-uefi
├── rpi5-uefi-master.zip   # UEFI 源码压缩包（本地备份）
├── RPi5_UEFI_Release_v0.3/ # 编译好的固件（本地）
├── UEFI/                   # UEFI 相关代码
└── *.docx / *.xlsx        # 论文文档
```

## 同步规则

| 文件类型 | 同步方式 |
|---------|---------|
| 代码、.md | GitHub（git 追踪） |
| Word、Excel、固件 | GitHub（**Git LFS** 追踪） |
| 大型二进制文件 | GitHub（**Git LFS** 追踪） |

### Git LFS 配置

已配置 Git LFS 追踪以下文件类型：
- `*.docx` `*.xlsx` `*.xls` `*.doc`（Office 文档）
- `*.zip` `*.rar`（压缩包）
- `*.fd` `*.efi`（固件文件）

**Windows AI 注意事项**：
- 提交大文件时需要安装 Git LFS：`git lfs install`
- 首次 push 时会触发 LFS 上传
- 不需要手动运行 `git lfs push`，正常 `git add` + `git commit` + `git push` 即可

---

## 注意事项

1. 工程在桌面，不在 OneDrive
2. 遇到 README.md 先确认来源（上游 vs 用户自己的）
3. 大文件由 Git LFS 管理，正常 git add/commit/push 即可
4. 每次对话结束后更新对话记录
