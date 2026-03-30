# Git 多用户协作与 SSH 配置

本文记录在 VMware 虚拟机环境下配置 Git 多用户协作的过程，包括 SSH key 生成、Git 常用命令详解、多 AI 协作分工等内容。

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

## 四、Git 常用命令详解

### 4.1 仓库初始化与克隆

```bash
git init              # 在当前目录初始化新仓库
git clone <url>       # 克隆远程仓库到本地
git clone <url> ~/dir # 克隆到指定目录
```

**使用场景**：新项目开始时，或参与已有项目时使用。

### 4.2 查看状态与历史

```bash
git status                        # 查看工作区状态（已修改/已暂存/未跟踪）
git status -s                     # 简洁格式输出
git diff                          # 查看未暂存的修改内容
git diff --staged                 # 查看已暂存的修改内容
git diff <file>                   # 查看指定文件的修改
git log                           # 查看完整提交历史
git log --oneline                 # 查看简洁的提交历史（一行一条）
git log --graph --oneline --all   # 查看分支图
git show <commit>                 # 查看某次提交的具体内容
```

**使用场景**：
- `git status`：每次提交前必看，确认修改范围
- `git diff`：提交前检查修改是否正确
- `git log`：追溯问题、查看项目演进历史

### 4.3 暂存与提交

```bash
git add <file>              # 暂存指定文件
git add .                   # 暂存所有修改（不包括删除）
git add -A                  # 暂存所有修改（包括删除）
git add -p                  # 交互式暂存，逐个修改选择是否暂存
git commit -m "message"     # 提交，消息描述本次修改
git commit -a -m "message"  # 自动暂存已跟踪文件并提交（跳过 git add）
git commit --amend          # 修改最后一次提交（补交文件/改消息）
```

**使用场景**：
- `git add`：确定哪些修改要纳入本次提交
- `git commit -m`：简洁提交，描述做了什么
- `git commit -a -m`：快速提交已跟踪文件的修改
- `git commit --amend`：最后一次提交漏了文件或消息写错时使用

### 4.4 分支管理

```bash
git branch                    # 列出本地所有分支
git branch -r                # 列出远程所有分支
git branch -a                # 列出所有分支（本地+远程）
git branch <name>            # 创建新分支
git branch -d <name>         # 删除分支（已合并）
git branch -D <name>         # 强制删除分支
git branch -m <old> <new>   # 重命名分支
```

**使用场景**：开发新功能、修复 bug 时创建独立分支，避免污染主分支。

### 4.5 切换分支

```bash
git checkout <branch>        # 切换到指定分支
git checkout -b <branch>     # 创建并切换到新分支
git checkout -b <branch> <remote>/<branch>  # 基于远程分支创建本地分支
git switch <branch>          # 切换分支（Git 2.23+ 推荐）
git switch -c <branch>       # 创建并切换（Git 2.23+）
git switch -                  # 切换到上一个分支
```

**使用场景**：
- `git checkout -b`：开始新功能开发时创建独立分支
- `git switch`：在分支间快速切换

### 4.6 合并分支

```bash
git merge <branch>           # 将指定分支合并到当前分支
git merge --no-ff <branch>   # 禁用快进合并，保留分支历史
git merge --squash <branch>  # 压缩合并，所有提交合成一个
```

**使用场景**：
- `git merge`：功能开发完成后合并回主分支
- `--no-ff`：需要保留分支历史时使用
- `--squash`：清理分支上的多个小提交，合并成一个大提交

**冲突处理**：
1. 手动编辑冲突文件
2. `git add <file>` 标记为已解决
3. `git commit` 完成合并

### 4.7 拉取与推送

```bash
git fetch                    # 获取远程最新状态（不合并）
git fetch <remote> <branch>  # 获取指定远程分支
git pull                    # 获取并合并远程更新（相当于 fetch + merge）
git pull --rebase           # 使用变基代替合并
git push                    # 推送到远程
git push -u <remote> <branch>  # 推送并设置上游分支
git push --force            # 强制推送（谨慎使用）
```

**使用场景**：
- `git fetch`：先看远程有什么更新，再决定是否合并
- `git pull`：确认远程有更新时同步到本地
- `git push -u`：首次推送分支到远程，后续可直接 `git push`

### 4.8 储藏工作区

```bash
git stash                    # 暂存当前工作区修改
git stash list              # 查看储藏列表
git stash pop               # 恢复最新储藏并删除
git stash apply             # 恢复储藏但保留副本
git stash drop              # 删除储藏
git stash clear             # 清空所有储藏
```

**使用场景**：临时切换分支处理紧急事务，不想提交半成品修改时使用。

### 4.9 重置与回退

```bash
git reset --soft HEAD~1     # 撤销上次提交，保留修改在暂存区
git reset --mixed HEAD~1    # 撤销上次提交，保留修改在工作区（默认）
git reset --hard HEAD~1     # 撤销上次提交，丢弃所有修改（危险）
git reset --hard <commit>   # 回退到指定提交
git revert <commit>         # 创建新提交反转指定提交的修改
```

**使用场景**：
- `git reset --soft`：提交后发现漏了文件，补上
- `git reset --hard`：确定不要某些提交时使用（不可逆）
- `git revert`：已推送的提交需要撤销时使用，安全且保留历史

### 4.10 变基

```bash
git rebase <branch>         # 将当前分支变基到目标分支
git rebase -i HEAD~3        # 交互式变基，修改最近 3 个提交
```

变基操作：
- `pick`：保留该提交
- `squash`：与前一个提交合并
- `reword`：修改提交消息
- `drop`：删除提交

**使用场景**：
- 整理提交历史，把多个小提交合成一个
- 保持分支历史线性
- **注意**：不要对已推送的提交进行变基

### 4.11 标签管理

```bash
git tag                     # 列出所有标签
git tag <name>             # 创建轻量标签
git tag -a <name> -m "msg" # 创建附注标签（推荐）
git tag -d <name>          # 删除本地标签
git push <remote> <tag>    # 推送标签到远程
git push <remote> --tags  # 推送所有标签
```

**使用场景**：版本发布时打标签，如 `v1.0.0`、`Release-v0.3`。

### 4.12 远程仓库管理

```bash
git remote -v                    # 查看远程仓库地址
git remote add <name> <url>      # 添加远程仓库
git remote set-url <name> <url>  # 修改远程仓库地址
git remote remove <name>         # 删除远程仓库
git remote -vv                   # 查看远程仓库详细信息
```

**使用场景**：
- 关联多个远程仓库（如 GitHub + Gitee）
- HTTPS 转 SSH 时修改地址

---

## 五、多用户协作与版本管理

### 5.1 协作模式：集中式 vs 分支式

**集中式**：所有人共享一个分支，提交前先 `pull` 保持同步。适合小团队。

**分支式**：每人一个功能分支，开发完通过 PR/MR 合并。适合多 AI 协作。

### 5.2 分支策略（Git Flow）

```
main/master    ──────────────────────── 主分支，稳定版本
                      ↑
release        ←─────────────────────── 发布分支
                      ↑
develop        ←─────────────────────── 开发分支
                      ↑
feature/xxx    ←─────────────────────── 功能分支（开发完合并回 develop）
hotfix/xxx     ←─────────────────────── 紧急修复分支（直接合并到 main 和 develop）
```

对于毕业设计项目，简化如下：

```
main            ──────────────────────── 主分支，稳定版本
                      ↑
feature/xxx    ←─────────────────────── 功能分支（AI 协作时各 AI 一个分支）
```

### 5.3 多 AI 协作流程

本项目采用双 AI 协作模式：

**分工**：
| AI 端 | 职责 | 分支 |
|-------|------|------|
| Linux AI | 代码、版本管理 | main |
| Windows AI | Word/Excel 大文件 | 本地修改，不进 git |

**工作流程**：

```
1. Linux AI 创建功能分支
   git checkout -b feature/blog-git

2. Linux AI 在分支上开发
   git add .
   git commit -m "添加博客文章"

3. Linux AI 切换到 main，拉取最新代码
   git checkout main
   git pull origin main

4. Linux AI 合并功能分支
   git merge feature/blog-git

5. Linux AI 推送 main
   git push origin main
```

### 5.4 推送前检查清单

```bash
# 1. 查看当前状态
git status

# 2. 查看修改内容
git diff

# 3. 查看提交历史（确认要提交的内容）
git log --oneline -5

# 4. 确认远程分支情况
git branch -a
```

### 5.5 处理冲突

多人协作时可能产生冲突：

```bash
# 1. 先拉取远程更新
git fetch origin

# 2. 合并时发现冲突
git merge origin/main
# 冲突文件会出现 <<<<<<< HEAD ... >>>>>>> 标记

# 3. 手动编辑冲突文件，删除标记，保留正确内容

# 4. 标记解决
git add <resolved-file>

# 5. 完成合并提交
git commit -m "合并分支：解决冲突"
```

### 5.6 提交规范

推荐提交消息格式：

```
<类型>: <简短描述>

<可选的详细说明>

<可选的脚注>
```

类型示例：
- `feat:` 新功能
- `fix:` 修复 bug
- `docs:` 文档更新
- `refactor:` 重构
- `chore:` 杂项（构建、工具配置等）

示例：
```
feat: 添加 Git 多用户协作博客

记录 SSH 配置、多 AI 协作分工等内容

Co-Authored-By: Claude Opus 4.6 <noreply@anthropic.com>
```

---

## 六、遇到的问题与解决

### 6.1 HTTPS 推送需要密码

错误：`could not read Username for 'https://github.com': No such device or address`

解决：改用 SSH 方式，配置 SSH key 后切换 remote：

```bash
git remote set-url origin git@github.com:username/repository.git
```

### 6.2 作者身份未知

错误：`Author identity unknown`

解决：

```bash
git config user.email "your_email@example.com"
git config user.name "your_username"
```

### 6.3 SSH Host key verification failed

错误：`Host key verification failed`

解决：将 GitHub 的 host key 添加到 known_hosts：

```bash
ssh-keyscan github.com >> ~/.ssh/known_hosts
```

### 6.4 合并冲突

多人协作时同一文件被不同人修改会产生冲突，参见本文 5.5 节处理方法。

---

## 七、延伸

- [GitHub SSH 文档](https://docs.github.com/en/authentication/connecting-to-github-with-ssh)
- [Git 官方文档](https://git-scm.com/book/zh/v2)
- [Git Flow 工作流](https://nvie.com/posts/a-successful-git-branching-model/)
