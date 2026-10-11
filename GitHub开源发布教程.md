# GitHub 开源发布教程 —— 三角洲行动玩法风格推荐器（DFA Advisor）

> 本教程从 **零基础** 出发，手把手带你把这个 VS2026 多工程 C++ 项目发布到 GitHub：
> 安装 Git → 本地首次提交 → 创建 GitHub 仓库 → 推送 → 选择 License → 用 Releases 发布 exe。
> 全程约 20 分钟。每一步都给出可直接复制的命令。

---

## 目录

- [第 0 章 前置准备：安装 Git](#第-0-章-前置准备安装-git)
- [第 1 章 在 GitHub 创建远程仓库](#第-1-章-在-github-创建远程仓库)
- [第 2 章 本地初始化并首次提交](#第-2-章-本地初始化并首次提交)
- [第 3 章 关联远程仓库并推送](#第-3-章-关联远程仓库并推送)
- [第 4 章 选择开源 License（许可证）](#第-4-章-选择开源-license许可证)
- [第 5 章 用 Releases 发布 exe（别人可直接下载运行）](#第-5-章-用-releases-发布-exe别人可直接下载运行)
- [第 6 章 日常迭代维护](#第-6-章-日常迭代维护)
- [第 7 章 常见问题排查](#第-7-章-常见问题排查)
- [附录：常用 Git 命令速查](#附录常用-git-命令速查)

---

## 第 0 章 前置准备：安装 Git

Git 是版本管理工具，GitHub 依赖它上传/下载代码。本机**尚未安装**，请先安装。

### 0.1 下载并安装

1. 打开官网：<https://git-scm.com/download/win>
2. 下载 **64-bit** 版本，双击安装包，一路点 **Next**（全部默认即可，无需改任何选项）
3. 安装完成后**关闭并重新打开**命令提示符 / PowerShell（让环境变量生效）

### 0.2 验证安装

```bash
git --version
```

看到类似 `git version 2.4x.x.windows.1` 即安装成功。

### 0.3 设置你的身份（首次必做，用于提交记录署名）

```bash
git config --global user.name "你的GitHub用户名"
git config --global user.email "你的GitHub注册邮箱"
```

> 例：`git config --global user.name "DeltaAdvisor"`

---

## 第 1 章 在 GitHub 创建远程仓库

1. 打开 <https://github.com> 并**登录**（没账号先注册，免费）
2. 右上角 **+** → **New repository**
3. 填写：
   - **Repository name**：`DeltaForce-Advisor`（或你喜欢的名字）
   - **Description**（可选）：`三角洲行动·烽火地带 玩法风格推荐器（离线 C++ 工具，多工程模块化）`
   - **Public**（公开，别人可见）/ **Private**（私有，仅自己可见）→ 开源选 **Public**
   - ⚠️ **不要勾选** Add a README / .gitignore / License —— 这三个我们本地已经有，勾了会冲突
4. 点 **Create repository**
5. 创建后页面会显示一段 `git remote add origin ...` 的命令，**先复制留着**（第 3 章用）

> 页面上的其他命令**不要执行**，我们按本教程走（你的本地目录已有全部文件，不需要从头建）。

---

## 第 2 章 本地初始化并首次提交

> 项目位置：`C:\Users\123\Desktop\DFA_ModularProjects`
> 已为你准备好 `.gitignore`（自动忽略 `.vs/`、`bin/`、`obj/` 等构建产物，只提交源码）。

### 2.1 打开命令行并进入项目目录

```bash
cd C:\Users\123\Desktop\DFA_ModularProjects
```

### 2.2 初始化仓库

```bash
git init
```

> 成功后会显示 `Initialized empty Git repository in ...`

### 2.3 把全部文件加入暂存区

```bash
git add .
```

### 2.4 确认提交内容（关键检查步骤）

```bash
git status
```

检查要点：
- 应看到：`Common`、`DFA_Weapons`、`DFA_Operators`、`DFA_Maps`、`DFA_Economy`、`DFA_Styles`、`DFA_Engine`、`DFA_Main`、`DeltaForceAdvisor.sln`、`README.md`、`.gitignore` 等
- **不应出现**：`bin/`、`obj/`、`.vs/`、`*.lib`、`*.pdb`、`DeltaForceAdvisor.exe`、`*.zip` —— 这些已被 .gitignore 排除
- 如果 `bin` 等仍出现在列表里，说明有残留，执行：`git rm -r --cached bin obj .vs` 后再 `git add .`

### 2.5 提交（打上第一个版本记录）

```bash
git commit -m "feat: 三角洲行动玩法风格推荐器 v2.2（多工程模块化）"

# 说明：7 个 VS 工程（枪械/干员/地图/经济/玩法/引擎/主程序）
# 69 把枪械、17 名干员、6 图 4 难度、五档玩法推荐
```

---

## 第 3 章 关联远程仓库并推送

### 3.1 关联远程地址（用第 1 章复制的命令）

```bash
git remote add origin https://github.com/你的用户名/仓库名.git
```

### 3.2 确保主分支名为 main

```bash
git branch -M main
```

> 现在 GitHub 默认主分支叫 `main`，本地仓库默认可能叫 `master`，这条命令统一为 `main`。

### 3.3 推送（首次推送会要求登录 GitHub）

```bash
git push -u origin main
```

> 弹出浏览器登录窗口时，用你的 GitHub 账号登录授权即可（推荐选 VS Code / 浏览器登录方式）。

### 3.4 验证

刷新 GitHub 仓库页面，应能看到全部源码文件。**发布成功！** 🎉

---

## 第 4 章 选择开源 License（许可证）

License 决定别人能否使用/修改/分发你的代码。本项目已含游戏相关数据，建议：

### 方案 A（推荐）：MIT License —— 最宽松、最常见

1. 在项目目录新建文件 `LICENSE`
2. 粘贴 MIT 模板，把 `[year]` 改成 2026、`[fullname]` 改成你的名字：

```text
MIT License

Copyright (c) 2026 你的名字

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

3. 提交并推送：

```bash
git add LICENSE
git commit -m "docs: 添加 MIT License"
git push
```

### 方案 B：加免责声明（推荐与 A 同时做）

在 README.md 顶部已声明：
> 本项目与《三角洲行动》官方无任何关联，数据整理自公开资料，非官方数据，仅供参考。

这样既开源代码，又规避游戏素材版权争议。**代码部分用 MIT，游戏数据仅作学习交流引用。**

---

## 第 5 章 用 Releases 发布 exe（别人可直接下载运行）

GitHub 仓库只存源码；**exe 通过 Releases 发布**，访问者一键下载即可运行。

### 5.1 打好版本标签（tag）

```bash
git tag v1.0.0
git push origin v1.0.0
```

### 5.2 准备 exe 发布包

1. 用 VS2026 打开 `DeltaForceAdvisor.sln`
2. 配置 **Release | x64** → `生成 → 重新生成解决方案`
3. 取最新 exe：`DFA_Main\bin\x64\Release\DFA_Main.exe`
4. 新建文件夹 `release_v1.0.0`，放入：
   - `DeltaForceAdvisor.exe`（把 DFA_Main.exe 重命名）
   - 运行说明（可复制 README 的「编译与运行 / 数据口径」章节）
5. 压缩为 `DeltaForceAdvisor_v1.0.0_win64.zip`

### 5.3 在 GitHub 发布 Release

1. 仓库页面 → 右侧 **Releases** → **Create a new release**（或 Draft a new release）
2. **Choose a tag**：选 `v1.0.0`
3. **Title**：`v1.0.0 初版发布`
4. **描述**：写更新内容，例如：

```text
三角洲行动·烽火地带 玩法风格推荐器 v1.0.0

- 输入：枪械 / 干员 / 地图 / 难度 / 哈夫币存量 / 固定队友
- 输出：跑刀 · 当鼠鼠 · 正常玩 · 微交战 · 全装猛攻（评分 + 判断依据 + 玩法档案 + 风险提示）
- 7 个 VS2026 模块化工程，数据集中在各模块「配置区」，方便扩展
- exe 为 /MT 静态链接，Win10/11 x64 直接双击运行，无需安装任何运行库
```

5. 把 `DeltaForceAdvisor_v1.0.0_win64.zip` 拖入 **Attach binaries**
6. 点 **Publish release**

### 5.4 完成效果

- Release 页面出现下载入口，任何人可下载 zip 直接运行
- 可在 README 顶部加一行徽章式链接（可选）：

```markdown
## 下载
[📥 最新版本 Releases](https://github.com/你的用户名/仓库名/releases)
```

---

## 第 6 章 日常迭代维护

以后每次改代码 / 调数据，只需三步：

```bash
cd C:\Users\123\Desktop\DFA_ModularProjects
git add .
git commit -m "feat: 更新xxx（一句话说明改了什么）"
git push
```

发布新版本时：

```bash
git tag v1.0.1          # 新版本号
git push origin v1.0.1  # 推送标签
# 然后在 GitHub Releases 页面新建 Release 上传新 exe 包
```

---

## 第 7 章 常见问题排查

| 问题 | 原因与解决 |
|---|---|
| `git push` 要求登录 | 正常，首次推送会弹窗授权，选浏览器登录即可 |
| 推送失败 `rejected` | 远程仓库已有内容（如勾选了 README）。先 `git pull origin main --allow-unrelated-histories` 合并，再 push |
| `bin/` 被提交了 | 执行 `git rm -r --cached bin obj .vs` 取消跟踪，再 `git add . && git commit -m "fix: 移除构建产物" && git push` |
| 中文文件名显示乱码 | 设置 `git config --global core.quotepath false` |
| 换行符警告（CRLF） | 正常提示，不影响使用；可用 `git config core.autocrlf true` 消除 |
| exe 太大传不上去 | exe 本来就不进仓库，用 Releases 发布（见第 5 章） |
| 忘了提交身份 | `git config --global user.name/email` 设置后重新 commit |
| 想撤销上一次提交 | `git reset --soft HEAD~1`（保留改动）或 `git reset --hard HEAD~1`（丢弃改动，慎用） |

---

## 附录：常用 Git 命令速查

```bash
git status                 # 查看当前改动状态
git add .                  # 暂存全部改动
git commit -m "说明"       # 提交
git push                   # 推送到远程
git pull                   # 拉取远程更新
git log --oneline          # 查看提交历史
git tag                    # 查看标签
git diff                   # 查看未暂存的改动内容
git branch -a              # 查看所有分支
```

---

## 一键流程总结（复制即用）

```bash
# ① 安装 Git 后执行一次
git config --global user.name "你的用户名"
git config --global user.email "你的邮箱"

# ② 发布流程
cd C:\Users\123\Desktop\DFA_ModularProjects
git init
git add .
git commit -m "feat: 三角洲行动玩法风格推荐器 v2.2"
git branch -M main
git remote add origin https://github.com/你的用户名/仓库名.git
git push -u origin main

# ③ 打版本 + 发布 exe
git tag v1.0.0 && git push origin v1.0.0
# 然后到 GitHub 页面 Releases → New release → 上传 exe zip
```
