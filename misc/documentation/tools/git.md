`@DOCS [tag: git, command] [description: Cheat sheet for Git commands.]`

### Git commands (Basics & Workflow)

* `git init` - Initialize a new Git repository in the current directory.
* `git clone <repository-url>` - Clone an existing Git repository from a remote URL.
* `git status` - Show the status of changes in the working directory and staging area.
* `git add <file>` - Add a file to the staging area.
* `git commit -m "commit message"` - Commit staged changes with a descriptive message.
* `git push origin <branch>` - Push local commits to the remote repository on the specified branch.
* `git pull origin <branch>` - Fetch and merge changes from the remote repository to the local branch.
* `git branch` - List all branches in the repository.
* `git checkout <branch>` - Switch to a different branch.
* `git merge <branch>` - Merge the specified branch into the current branch.

### Git commands to investigate history

* `git log` - Show the commit history of the current branch.
* `git reflog` - Show the history of all actions (commits, checkouts, etc.) in the repository.
* `git diff` - Show changes between commits, branches, or the working directory and staging area.
* `git show <commit-hash>` - View the specific changes and metadata of a single commit.
* `git blame <file>` - Show the last modification for each line in a file, along with the author and commit information.

### Undoing changes and "Oops" commands

* `git reset --soft HEAD~1` - Undo the last commit but keep your changes in the staging area.
* `git reset --hard HEAD~1` - Completely discard the last commit and all changes associated with it. **(Use with caution!)**
* `git checkout -- <file>` - Discard local changes in a specific file and restore it to the last committed state.
* `git revert <commit-hash>` - Create a new commit that inverses the changes of a previous commit (safest for shared branches).
* `git stash` - Temporarily shelf current changes so you can switch branches without committing.
* `git stash pop` - Bring back your stashed changes and remove them from the stash list.

### Remote and Branch Management

* `git remote -v` - List all remote repositories linked to your local project.
* `git branch -d <branch-name>` - Delete a local branch (use `-D` to force delete unmerged branches).
* `git fetch` - Download objects and refs from another repository without merging them into your local work.
* `git cherry-pick <commit-hash>` - Apply a specific commit from one branch onto your current branch.

