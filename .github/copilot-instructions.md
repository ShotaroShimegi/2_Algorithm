# GitHub Copilot Instructions

## Git コマンド実行ルール

### 要承認コマンド（実行前に必ず確認を求める）

以下のコマンドは、リポジトリの状態やリモートに影響を与えるため、実行前に必ずユーザーに確認すること：

- `git push` (すべてのバリエーション)
- `git pull` (マージが発生する可能性)
- `git commit` (コミット作成)
- `git merge` (ブランチのマージ)
- `git rebase` (履歴の書き換え)
- `git reset` (HEAD の移動)
- `git checkout -b` (新しいブランチの作成)
- `git branch -d` / `git branch -D` (ブランチの削除)
- `git tag` (タグの作成・削除)
- `git stash drop` (スタッシュの削除)
- `git clean` (ファイルの削除)
- `git remote add` / `git remote remove` (リモートの追加・削除)
- `git submodule` (サブモジュール操作)

### 自動実行可能コマンド（承認不要）

以下の読み取り専用コマンドは、自動的に実行してよい：

- `git status` (状態確認)
- `git log` (履歴表示)
- `git diff` (差分表示)
- `git show` (コミット詳細表示)
- `git branch` (ブランチ一覧表示、削除以外)
- `git remote -v` (リモート一覧表示)
- `git config --list` (設定表示)
- `git ls-files` (ファイル一覧)
- `git blame` (履歴追跡)
- `git reflog` (参照ログ)
- `git fetch` (取得のみ、マージしない)
- `git --version` (バージョン確認)

## 実行時の指示

1. **要承認コマンドの場合**:
   - コマンドの内容と影響を説明
   - 実行してよいか明示的に確認
   - ユーザーの承認後に実行

2. **自動実行コマンドの場合**:
   - 簡潔な説明と共にそのまま実行
   - 結果を確認して次のステップを提案

3. **複数コマンドの実行**:
   - 要承認コマンドが含まれる場合は、そのコマンドの前で一度停止
   - 読み取りコマンドは連続して実行可能

## 作業フロー例

```
ユーザー: 「リポジトリの状態を確認して、変更をプッシュして」

Copilot の動作:
1. git status (自動実行) → 結果表示
2. git log --oneline -5 (自動実行) → 最新履歴表示
3. git diff (自動実行) → 差分があれば表示
4. 「以下のコマンドを実行してプッシュしますか？」と確認
   - git add .
   - git commit -m "..."
   - git push
5. ユーザー承認後に実行
```
