# Issue tracker: GitHub

Issues 和 PRD 以 GitHub Issues 形式存放在 [AUR0RACY/quantitative-platform](https://github.com/AUR0RACY/quantitative-platform)。所有操作使用 `gh` CLI。

## 约定

- **创建 Issue**: `gh issue create --title "..." --body "..."`。多行正文使用 heredoc。
- **读取 Issue**: `gh issue view <number> --comments`，通过 `jq` 过滤评论并获取标签。
- **列出 Issue**: `gh issue list --state open --json number,title,body,labels,comments --jq '[.[] | {number, title, body, labels: [.labels[].name], comments: [.comments[].body]}]'`，配合 `--label` 和 `--state` 过滤。
- **评论 Issue**: `gh issue comment <number> --body "..."`
- **添加/移除标签**: `gh issue edit <number> --add-label "..."` / `--remove-label "..."`
- **关闭 Issue**: `gh issue close <number> --comment "..."`

通过 `git remote -v` 推断仓库 — `gh` 在克隆仓库内运行时会自动处理。

## 当 skill 说"发布到 issue 追踪器"

创建 GitHub Issue。

## 当 skill 说"获取相关的工单"

运行 `gh issue view <number> --comments`。
