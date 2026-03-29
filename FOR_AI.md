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

## Git 仓库

```bash
git remote add origin git@github.com:airzam/Graduation-project.git
```

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

---

## 目录结构

```
毕业设计/
├── FOR_AI.md              # AI 操作历史记录（本文档）
├── README.md               # 项目总览
├── blog/                   # 技术博客
│   ├── README.md          # 博客写作规范
│   └── 01-*.md            # 博客文章
├── rpi5-uefi-master.zip   # UEFI 源码压缩包（本地备份）
├── RPi5_UEFI_Release_v0.3/ # 编译好的固件（本地）
├── UEFI/                   # UEFI 相关代码
└── *.docx / *.xlsx        # 论文文档
```

## 同步规则

| 文件类型 | 同步方式 |
|---------|---------|
| 代码、.md | GitHub |
| Word、Excel、固件 | Windows 本地 |

---

## 注意事项

1. 工程在桌面，不在 OneDrive
2. 遇到 README.md 先确认来源（上游 vs 用户自己的）
3. 大文件（docx/xlsx/固件）不放 GitHub
4. 每次对话结束后更新对话记录
