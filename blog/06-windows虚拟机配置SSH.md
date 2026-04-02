# Windows 虚拟机配置 SSH

## 背景

在 VMware 虚拟机中的 Ubuntu 系统上配置 SSH 服务，实现从 Windows 宿主机远程连接。

## 操作步骤

### 1. 安装 openssh-server

```bash
sudo apt update
sudo apt install -y openssh-server
```

### 2. 启动 SSH 服务

```bash
sudo systemctl enable ssh
sudo systemctl start ssh
sudo systemctl status ssh
```

### 3. 检查服务状态

```bash
sudo systemctl status ssh
```

输出显示：
```
● ssh.service - OpenBSD Secure Shell server
   Active: active (running)
```

### 4. 检查防火墙

```bash
sudo ufw status
```

防火墙未启用，可正常连接。

### 5. 查看虚拟机 IP 地址

```bash
hostname -I
```

输出：
```
192.168.186.135
```

## 连接方式

在 Windows 命令行或 PowerShell 中：

```bash
ssh airzam@192.168.186.135
```

密码：`123456`

## 相关配置

- SSH 配置文件：`/etc/ssh/sshd_config`
- SSH 服务端口：22
- 监听地址：0.0.0.0（所有网卡）
- 开机自启：已启用

## 参考

- [02-git多用户协作与SSH配置.md](./02-git多用户协作与SSH配置.md)
