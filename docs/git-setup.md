# Git リポジトリセットアップ手順

## ステップ1: ローカルリポジトリの初期化

PowerShellで以下のコマンドを実行：

```powershell
# 現在のディレクトリで Git リポジトリを初期化
git init

# 初回コミット前の設定確認（必要に応じて設定）
git config user.name "Your Name"
git config user.email "your.email@example.com"

# ファイルをステージング
git add .

# 初回コミット
git commit -m "Initial commit: Micromouse maze algorithm implementation"
```

## ステップ2: リモートリポジトリの作成

### GitHub の場合

1. https://github.com にアクセス
2. 右上の「+」→「New repository」をクリック
3. リポジトリ名を入力（例: `micromouse-maze`）
4. Description: "Maze algorithm for micromouse robot"
5. **重要**: "Initialize this repository with a README" のチェックを**外す**
6. 「Create repository」をクリック

### GitLab の場合

1. https://gitlab.com にアクセス
2. 「New project」→「Create blank project」
3. Project name: `micromouse-maze`
4. **重要**: "Initialize repository with a README" のチェックを**外す**
5. 「Create project」をクリック

## ステップ3: リモートリポジトリとの接続

作成したリポジトリのURLをコピーし、以下のコマンドを実行：

```powershell
# リモートリポジトリを追加（URLは自分のリポジトリに置き換える）
# GitHub の場合:
git remote add origin https://github.com/your-username/micromouse-maze.git

# GitLab の場合:
# git remote add origin https://gitlab.com/your-username/micromouse-maze.git

# デフォルトブランチ名を main に設定（推奨）
git branch -M main

# 初回プッシュ
git push -u origin main
```

## ステップ4: SSH接続の設定（推奨・オプション）

HTTPSではなくSSHで接続する場合（より安全）：

```powershell
# SSH鍵の生成（既に持っている場合はスキップ）
ssh-keygen -t ed25519 -C "your.email@example.com"
# Enter キーを3回押す（デフォルト設定）

# 公開鍵をクリップボードにコピー
Get-Content $HOME\.ssh\id_ed25519.pub | Set-Clipboard

# GitHub/GitLabの設定ページでSSH鍵を追加
# GitHub: Settings → SSH and GPG keys → New SSH key
# GitLab: Preferences → SSH Keys

# リモートURLをSSHに変更
git remote set-url origin git@github.com:your-username/micromouse-maze.git
# または GitLab: git@gitlab.com:your-username/micromouse-maze.git
```

## ステップ5: サブモジュールとしての組み込み準備

このリポジトリを別のプロジェクトで使用する場合：

```powershell
# 親プロジェクトのディレクトリで実行
cd path\to\your-micromouse-robot

# サブモジュールとして追加
git submodule add https://github.com/your-username/micromouse-maze.git lib/maze

# サブモジュールの初期化と更新
git submodule update --init --recursive

# コミット
git add .gitmodules lib/maze
git commit -m "Add maze algorithm as submodule"
```

## ステップ6: 日常的な作業フロー

```powershell
# 変更をステージング
git add .

# コミット
git commit -m "Update: 機能の説明"

# リモートにプッシュ
git push

# リモートから最新版を取得
git pull
```

## ブランチ戦略（推奨）

```powershell
# 開発用ブランチを作成
git checkout -b develop

# 機能追加用ブランチ
git checkout -b feature/new-algorithm

# 作業後、developにマージ
git checkout develop
git merge feature/new-algorithm

# リリース前にmainにマージ
git checkout main
git merge develop
git tag -a v1.0.0 -m "Release version 1.0.0"
git push origin main --tags
```

## トラブルシューティング

### プッシュ時に認証エラーが出る場合

Windows では、資格情報マネージャーが古いパスワードを保持している可能性があります：

```powershell
# 資格情報を削除
cmdkey /list | Select-String "git" | ForEach-Object { cmdkey /delete:($_ -replace ".*Target: ","") }

# 再度プッシュを試行（パスワードを再入力）
git push
```

### GitHubでPersonal Access Token (PAT)を使用する場合

1. GitHub → Settings → Developer settings → Personal access tokens → Tokens (classic)
2. "Generate new token" → "Generate new token (classic)"
3. スコープ: `repo` を選択
4. トークンをコピー
5. プッシュ時、パスワードの代わりにトークンを使用

## 参考情報

- [Git公式ドキュメント](https://git-scm.com/doc)
- [GitHub Docs](https://docs.github.com/)
- [Git Submodules](https://git-scm.com/book/en/v2/Git-Tools-Submodules)
