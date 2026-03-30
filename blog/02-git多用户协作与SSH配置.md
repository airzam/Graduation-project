# Git 多用户协作与 SSH 配置

本文记录在 VMware 虚拟机环境下配置 Git 多用户协作的过程，包括 SSH key 生成、仓库克隆、协作分工等内容。

## 一、SSH Key 配置

### 1.1 生成 SSH Key

```bash
ssh-keygen -t ed25519 -C "your_email@example.com" -f ~/.ssh/id_ed25519 -N ""
```

参数说明：
- `-t ed25519`：使用 Ed25519 算法，比 RSA 更安全且更短
- `-C`：注释，通常填邮箱
- `-f`：私钥文件路径
- `-N ""`：密码为空（虚拟机环境可省略密码）

### 1.2 添加到 GitHub

```bash
cat ~/.ssh/id_ed25519.pub
```

复制输出的公钥，添加到 GitHub → Settings → SSH and GPG keys → New SSH key。

### 1.3 验证连接

```bash
ssh -T git@github.com
```

成功会显示：`Hi xxx! You've successfully authenticated, but GitHub does not provide shell access.`

### 1.4 解决 Host key verification failed

如果遇到此错误：

```bash
ssh-keyscan github.com >> ~/.ssh/known_hosts
```

## 二、Git 基本配置

### 2.1 克隆仓库

```bash
git clone git@github.com:username/repository.git ~/directory
```

### 2.2 配置用户信息

```bash
git config user.email "your_email@users.noreply.github.com"
git config user.name "your_username"
```

### 2.3 查看配置

```bash
git config --list
```

## 三、多用户协作模式

### 3.1 分工设计

| 职责 | 执行者 |
|------|--------|
| 代码更新、版本管理、git 推送 | Linux AI |
| 大文件修改（Word、Excel 等） | Windows AI |

### 3.2 工作流程

```
Windows AI 本地修改完成
        ↓
通知 Linux AI 同步状态
        ↓
Linux AI 拉取最新代码
Linux AI 推送更新到 GitHub
```

## 四、常用 Git 命令

### 4.1 查看状态

```bash
git status          # 查看工作区状态
git remote -v       # 查看远程仓库地址
git log --oneline   # 查看提交历史
```

### 4.2 提交与推送

```bash
git add <file>                    # 暂存文件
git commit -m "提交说明"          # 提交
git push                          # 推送到远程
```

### 4.3 远程仓库操作

```bash
git remote set-url origin git@github.com:username/repository.git  # 切换远程地址（HTTPS → SSH）
git pull                           # 拉取远程更新
git fetch                          # 获取远程分支（不合并）
```

## 五、遇到的问题与解决

### 5.1 HTTPS 推送需要密码

错误：`could not read Username for 'https://github.com': No such device or address`

解决：改用 SSH 方式，配置 SSH key 后切换 remote：

```bash
git remote set-url origin git@github.com:username/repository.git
```

### 5.2 作者身份未知

错误：`Author identity unknown`

解决：

```bash
git config user.email "your_email@example.com"
git config user.name "your_username"
```

## 六、延伸

- [GitHub SSH 文档](https://docs.github.com/en/authentication/connecting-to-github-with-ssh)
- [Git 基本操作](https://git-scm.com/book/zh/v2)
