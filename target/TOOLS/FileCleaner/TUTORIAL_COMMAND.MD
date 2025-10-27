## HELP
*Show help information, short description on how commands and options work*

`cleaner help` : Show general help information
`cleaner help -commands` : Show shorer list of all supported commands
`cleaner <command> -help ` : Show detailed help information for a specific command


## DIR
*List files, similar to dir on windows or ls on linux but with other features*

`cleaner dir .` : List files in the current directory in table format
`cleaner ls .` : List files in the current directory and shows only files in rows below
`cleaner ls "**"` : List files in all subdirectories
`cleaner ls "**" --parents 2` : List files in all subdirectories and prints them with two parent folders
`cleaner dir "**" --pattern "<pattern>"` : List files in all subdirectories that contains the specified pattern
`cleaner dir "**" --rpattern "<pattern>"` : List files in all subdirectories that contains the specified regular expression pattern
`cleaner dir --source "**/test**"` : List files in all subdirectories with folder starting with "test"

## COPY
*Copy files from source to target directory with options to filter files on content and preview operations*

`cleaner copy . /user/username/test/,` : Copy files from the current directory to the specified target directory
`cleaner cp . /user/username/test/, --preview` : Preview copy operation without executing it
`cleaner copy "**" /user/username/test/ --filter "*.cpp"` : Copy all Java files from selected directory and subdirectories to the specified target directory and generate directories if not created


## COUNT

`cleaner count "**"` : Count lines, code lines, code characters, comments and strings in all files in current and subdirectories
`cleaner count "**" --pattern "assert"` : count all assert occurrences in all files in current and subdirectories
`cleaner count "**" --segment comment --pattern "TODO"` : count all TODO occurrences in all files in current and subdirectories within comments
`cleaner count "**" --segment comment --pattern "@PROJECT;@TASK;CODE;TODO"` : count all words between ; in comments

## LIST
*lists found rows with the pattern searched for*

`cleaner list "**"` : List all files in the current directory and subdirectories
`cleaner list "**" --filter "*.py" --pattern "<pattern>"` : List all Python files in the current directory and subdirectories
`cleaner list  "**" --pattern "logging" --filter "*.py"` : List all Python files in the current directory and subdirectories that contains the pattern "logging"
`cleaner list  "**" --pattern "logging;WARNING" --filter "*.py" -match-all` : List lines in Python files that contains the patterns "logging" and "WARNING"
`cleaner list  "**" --pattern "WARNING" --filter "*.py" -word -icase` : List lines in Python files that contains the word "WARNING" in any case format
`cleaner list  "**" --segment comment --pattern "WARNING" --filter "*.py" -word -icase` : List lines in Python files that contains the word "WARNING" in comments in any case format
`cleaner list  "**" --segment string --rpattern "TODO" --filter "*.js" -word -icase --context 5` : List lines JavaScript files that contains the word "TODO" in strings in any case format and preview 5 lines after found line

## FIND
*find works like `list` but it searches on all text, not only line based. `find` are able to match patterns across multiple lines and provide more context around the found matches.*

## HISTORY

`cleaner history -create -local` : Create a local history file in the current directory 

`cleaner find -R --filter "*.h*"  --segment comment --prompt "pattern" --add-to-history "c-pattern"` : Search for pattern that is asked for in comments and also add it to the history under the name "c-pattern"
`cleaner history "c-pattern"` : Run "c-pattern" in history to search for the saved command sequence

`cleaner history -print` : prints the local history file if found
`cleaner history -print -home` : prints the global history for user

`cleaner history -create -home` : Create a global history file for user
`cleaner history -remove --index <index>` : Remove a specific command from history by index
`cleaner history --index <index> --set-alias <alias>` : Set an alias for command and index in history

## CONFIG
*Manage configuration settings for FileCleaner tool*

`cleaner config -create -local` : Create a local configuration file in the current directory
`cleaner config -create -home` : Create a global configuration file for user
`cleaner config -print -local` : Print the local configuration file if found
`cleaner config -edit -local` : Tries to open local configuration file in default text editor