# GitHub Copilot Git ワークフロー設定ガイド

## 概要

このドキュメントでは、GitHub Copilotを使用したGit操作の自動化と安全性を両立させる設定方法を説明します。

## 設定方法

### 方法1: ワークスペース設定（推奨）

プロジェクト固有の設定として、`.vscode/settings.json` に以下を追加：

```json
{
  "github.copilot.chat.codeGeneration.instructions": [
    {
      "text": "Git操作の承認ルール: git pull, git push, git commit, git merge, git rebase, git reset, git branch -D など、リモートリポジトリやコミット履歴に影響を与えるコマンドは実行前に必ず確認を求めること。git status, git log, git diff, git branch, git remote -v などの読み取り専用コマンドは自動で実行してよい。"
    }
  ]
}
```

**メリット**:
- プロジェクトごとに設定可能
- チームで共有できる（Gitにコミット可能）
- サブモジュールとして使う場合も設定が引き継がれる

### 方法2: ユーザー設定（グローバル）

すべてのプロジェクトに適用する場合、VS Codeのユーザー設定に追加：

1. `Ctrl + Shift + P` → "Preferences: Open User Settings (JSON)"
2. 上記の設定を追加

**メリット**:
- すべてのプロジェクトで有効
- 一度設定すれば完了

### 方法3: GitHub Copilot Instructions（最も推奨）

`.github/copilot-instructions.md` ファイルを作成（このリポジトリに既に追加済み）

**メリット**:
- 最も詳細なルールを記述可能
- Copilotが自然言語で理解しやすい
- バージョン管理できる
- チームで共有しやすい

## 推奨する組み合わせ

**すべての方法を併用することを推奨**します：

```
1. .github/copilot-instructions.md  ← 詳細なルールと例
2. .vscode/settings.json            ← ワークスペース設定
3. ユーザー設定                      ← 個人のグローバル設定
```

優先順位: ワークスペース設定 > ユーザー設定

## Git操作の分類

### 🔴 要承認（危険な操作）

| コマンド | 理由 |
|---------|------|
| `git push` | リモートリポジトリに変更を送信 |
| `git pull` | マージが発生し、コンフリクトの可能性 |
| `git commit` | コミット履歴に影響 |
| `git merge` | ブランチの統合 |
| `git rebase` | 履歴の書き換え |
| `git reset` | HEADの移動、変更の破棄 |
| `git branch -D` | ブランチの強制削除 |
| `git stash drop` | スタッシュの削除 |
| `git clean` | 未追跡ファイルの削除 |
| `git tag -d` | タグの削除 |

### 🟢 自動実行可能（安全な操作）

| コマンド | 理由 |
|---------|------|
| `git status` | 読み取り専用 |
| `git log` | 履歴の表示のみ |
| `git diff` | 差分の表示のみ |
| `git show` | コミット情報の表示のみ |
| `git branch` | ブランチ一覧表示（削除以外） |
| `git remote -v` | リモート情報の表示のみ |
| `git fetch` | 取得のみ（マージしない） |
| `git ls-files` | ファイル一覧の表示のみ |
| `git blame` | 履歴追跡の表示のみ |

### 🟡 条件付き自動実行

| コマンド | 条件 |
|---------|------|
| `git add` | 新規ファイル追加時は自動OK、大量ファイル時は確認 |
| `git checkout` | ブランチ切り替えは自動OK、ファイル復元は確認 |
| `git stash` | 保存は自動OK、dropは確認必要 |

## 実際の使用例

### 例1: 状態確認からプッシュまで

```
ユーザー: 「変更をプッシュして」

Copilot:
1. [自動実行] git status
   → 変更ファイル一覧を表示
   
2. [自動実行] git diff
   → 変更内容を表示
   
3. [確認要求] 以下を実行しますか？
   - git add .
   - git commit -m "Update maze algorithm"
   - git push
   
4. ユーザーが承認
   
5. [実行] コマンドを順次実行
```

### 例2: リモートとの同期

```
ユーザー: 「リモートの最新を取得して」

Copilot:
1. [自動実行] git fetch
   → リモートの情報を取得
   
2. [自動実行] git log --oneline HEAD..origin/main
   → リモートとの差分を表示
   
3. [確認要求] リモートに新しいコミットがあります。
   git pull を実行しますか？
   
4. ユーザーが承認
   
5. [実行] git pull
```

### 例3: ブランチ操作

```
ユーザー: 「featureブランチを作成して切り替えて」

Copilot:
1. [自動実行] git branch
   → 現在のブランチ一覧を表示
   
2. [確認要求] 新しいブランチ 'feature/xxx' を作成して切り替えますか？
   - git checkout -b feature/xxx
   
3. ユーザーが承認
   
4. [実行] git checkout -b feature/xxx
```

## トラブルシューティング

### Copilotが設定を認識しない場合

1. VS Codeを再起動
2. Copilot拡張機能を再読み込み
3. 設定ファイルの構文エラーをチェック

### 設定が適用されているか確認

```powershell
# VS Code設定の確認
code --list-extensions | Select-String copilot

# settings.jsonの確認
Get-Content .vscode\settings.json
```

### Copilot Instructionsが読み込まれているか確認

Copilot Chatで以下を質問：
```
「このプロジェクトのGit操作ルールを教えて」
```

正しく設定されていれば、`.github/copilot-instructions.md` の内容に基づいて回答されます。

## 参考リンク

- [GitHub Copilot Instructions](https://docs.github.com/en/copilot/customizing-copilot/adding-custom-instructions-for-github-copilot)
- [VS Code Settings](https://code.visualstudio.com/docs/getstarted/settings)
- [Git Best Practices](https://git-scm.com/book/en/v2)
