# 毕业设计

## 文档

| 文件 | 说明 |
|------|------|
| 1.毕业设计开题报告.docx | 开题报告 |
| 2.文献综述模板.doc | 文献综述 |
| 3.毕业论文（设计）任务书.doc | 任务书 |
| 选题汇总表.xlsx | 选题汇总 |
| 中期检查表_v2.docx | 中期检查 |
| 附件2：毕业论文相关文件及表格 | 相关文件 |

## 任务

- [ ] 待办
- [ ] 进行中
- [x] 已完成：创建README.md，记录对话

## 工程目录

```
C:\Users\67426\OneDrive\毕业设计\
├── blog/              # 技术博客
│   ├── README.md      # 博客编写规范
│   └── 01-*.md        # 博客文章
├── .git/              # Git 仓库
└── ...
```

## 开发环境

- **AI 模型**：MiniMax M2.7
- **配置文件**：`~/.claude/settings.json`
- **API**：MiniMax 平台

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

## Git 仓库

```bash
git remote add origin git@github.com:airzam/Graduation-project.git
```

## 同步规则

| 文件类型 | 同步方式 |
|---------|---------|
| 代码、.md | GitHub |
| Word、Excel、固件 | Windows 本地（OneDrive 文件夹） |

## 备注

- GitHub 管理代码和博客（Linux push，Windows pull）
- Word/Excel 在 Windows 上直接修改（路径：`C:\Users\67426\OneDrive\毕业设计\`）
