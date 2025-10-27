## Help Commands
*Display usage info and command details*

- `cleaner help`: General overview
- `cleaner help -commands`: Command list
- `cleaner <cmd> -help`: Specific command details

## Dir/Ls
*Enhanced file listing (like ls/dir) with content filters*

- `cleaner dir .`: Table view of current dir
- `cleaner ls .`: Simple row list of current dir files
- `cleaner ls "**"`: Recursive file list (all subdirs)
- `cleaner ls "**" --parents 2`: Recursive with 2 parent paths
- `cleaner dir "**" --pattern "<pat>"`: Recursive files matching pattern
- `cleaner dir "**" --rpattern "<regex>"`: Recursive regex matches
- `cleaner dir --source "**/test**"`: Files in "test"-starting subdirs

## Copy/Cp
*Copy files with content filters and previews*

- `cleaner copy . /target/`: Copy current dir to target
- `cleaner cp . /target/ --preview`: Dry-run preview
- `cleaner copy "**" /target/ --filter "*.cpp"`: Recursive filtered copy (auto-mkdirs)

## Count
*Analyze lines/code/comments/strings or pattern occurrences*

- `cleaner count "**"`: Recursive line/code/comment stats
- `cleaner count "**" --pattern "assert"`: Count pattern occurrences
- `cleaner count "**" --segment comment --pattern "TODO"`: Count in comments
- `cleaner count "**" --segment comment --pattern "@PROJ;@TASK;TODO"`: Count semicolon-separated words in comments

## List
*Line-based pattern search with filters/segments*

- `cleaner list "**"`: Recursive file list
- `cleaner list "**" --filter "*.py" --pattern "<pat>"`: Filtered Python files
- `cleaner list "**" --pattern "logging" --filter "*.py"`: Python files with pattern
- `cleaner list "**" --pattern "logging;WARNING" --filter "*.py" -match-all`: Multi-pattern lines
- `cleaner list "**" --pattern "WARNING" --filter "*.py" -word -icase`: Case-insensitive whole-word matches
- `cleaner list "**" --segment comment --pattern "WARNING" --filter "*.py" -word -icase`: In comments only
- `cleaner list "**" --segment string --rpattern "TODO" --filter "*.js" -word -icase --context 5`: In strings + post-context

## Find
*Text-based search (non-line-bound; multi-line patterns, code-focused; use list for comments/strings)*

## History
*Command reuse and tracking*

- `cleaner history -create -local`: Init local history file
- `cleaner find -R --filter "*.h*" --segment comment --prompt "pat" --add-to-history "c-pat"`: Prompt/search + save as alias
- `cleaner history "c-pat"`: Run saved command
- `cleaner history -print`: View local history
- `cleaner history -print -home`: View global history
- `cleaner history -create -home`: Init global history
- `cleaner history -remove --index <n>`: Delete by index
- `cleaner history --index <n> --set-alias <alias>`: Alias by index

## Config
*Manage tool settings*

- `cleaner config -create -local`: Init local config
- `cleaner config -create -home`: Init global config
- `cleaner config -print -local`: View local config
- `cleaner config -edit -local`: Edit local config in default editor