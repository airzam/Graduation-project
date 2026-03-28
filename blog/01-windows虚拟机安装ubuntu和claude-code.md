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

### 6. 安装 VSCode

从 [VSCode 官网](https://code.visualstudio.com/) 下载 `.deb` 包，然后用 dpkg 安装：

```bash
sudo dpkg -i <下载的deb包路径>
```

### 7. 安装 Node.js

```bash
curl -fsSL https://deb.nodesource.com/setup_18.x | sudo -E bash -  # 添加 NodeSource 源
sudo apt install -y nodejs   # 安装 Node.js
```

### 8. 安装 Claude Code

```bash
sudo npm install -g @anthropic-ai/claude-code  # 全局安装 Claude Code
claude                                     # 启动 Claude Code
```

### 9. 配置 MiniMax M2.7 模型

使用国产 MiniMax M2.7 模型，降低使用成本。

#### 配置文件一：API 配置

```bash
nano ~/.claude/settings.json
```

```json
{
  "env": {
    "ANTHROPIC_BASE_URL": "https://api.minimaxi.com/anthropic",
    "ANTHROPIC_AUTH_TOKEN": "你的MiniMax API Key",
    "API_TIMEOUT_MS": "3000000",
    "CLAUDE_CODE_DISABLE_NONESSENTIAL_TRAFFIC": 1,
    "ANTHROPIC_MODEL": "MiniMax-M2.7",
    "ANTHROPIC_SMALL_FAST_MODEL": "MiniMax-M2.7",
    "ANTHROPIC_DEFAULT_SONNET_MODEL": "MiniMax-M2.7",
    "ANTHROPIC_DEFAULT_OPUS_MODEL": "MiniMax-M2.7",
    "ANTHROPIC_DEFAULT_HAIKU_MODEL": "MiniMax-M2.7"
  }
}
```

#### 配置文件二：主配置

```bash
nano ~/.claude.json
```

```json
{
  "hasCompletedOnboarding": true
}
```

从 [MiniMax 平台](https://platform.minimaxi.com/) 获取 API Key。
