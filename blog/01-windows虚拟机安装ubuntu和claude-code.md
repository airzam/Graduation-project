# 在 Windows 上搭建开发环境：虚拟机 + Ubuntu + Claude Code

## 所需软件

| 软件 | 下载地址 |
|------|----------|
| VMware Workstation Pro | https://www.vmware.com/products/workstation-pro.html |
| Ubuntu 桌面版 | https://ubuntu.com/download/desktop |
| Claude Code | https://docs.anthropic.com/claude-code |

硬件要求：CPU 支持虚拟化、内存 16GB+、硬盘 50GB+

## 安装步骤

### 1. BIOS 开启虚拟化

重启电脑，按 `F2` 或 `Del` 进入 BIOS，启用 `Intel Virtualization Technology` 或 `AMD-V`。

### 2. 创建虚拟机并安装 Ubuntu

1. VMware 新建虚拟机 → 选择 Ubuntu ISO 文件
2. 建议配置：4核、8GB内存、60GB硬盘
3. 安装完成后重启

### 3. 安装 VMware Tools

```bash
sudo apt update              # 更新软件包列表
sudo apt install -y open-vm-tools-desktop  # 安装 VMware Tools
reboot                       # 重启使生效
```

### 4. 换源（国内加速）

```bash
sudo cp /etc/apt/sources.list /etc/apt/sources.list.bak  # 备份原源
sudo nano /etc/apt/sources.list    # 编辑源列表
```

替换为阿里云源：

```
deb http://mirrors.aliyun.com/ubuntu/ jammy main restricted universe multiverse
deb http://mirrors.aliyun.com/ubuntu/ jammy-updates main restricted universe multiverse
deb http://mirrors.aliyun.com/ubuntu/ jammy-backports main restricted universe multiverse
deb http://mirrors.aliyun.com/ubuntu/ jammy-security main restricted universe multiverse
```

```bash
sudo apt update && sudo apt upgrade -y  # 更新软件包
```

### 5. 安装基础工具

```bash
sudo apt install -y curl wget git vim build-essential  # 安装常用工具
```

### 6. 安装 Node.js

```bash
curl -fsSL https://deb.nodesource.com/setup_18.x | sudo -E bash -  # 添加 NodeSource 源
sudo apt install -y nodejs   # 安装 Node.js
```

### 7. 安装 Claude Code

```bash
sudo npm install -g @anthropic-ai/claude-code  # 全局安装 Claude Code
claude                                     # 启动 Claude Code
```

首次运行按提示登录 Anthropic 账号并输入 API Key。

## 常用命令

| 命令 | 说明 |
|------|------|
| `claude` | 启动交互式对话 |
| `claude -p "问题"` | 非交互式提问 |
| `claude --help` | 查看帮助 |

## 配置 Git（可选）

```bash
git config --global user.name "你的名字"      # 设置用户名
git config --global user.email "你的邮箱"     # 设置邮箱
ssh-keygen -t ed25519 -C "你的邮箱"           # 生成 SSH 密钥
cat ~/.ssh/id_ed25519.pub  # 复制公钥到 GitHub SSH Keys
```
