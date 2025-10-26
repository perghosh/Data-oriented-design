**HELP**
`cleaner help` : Show general help information
`cleaner help -commands` : Show shorer list of all supported commands
`cleaner <command> -help ` : Show detailed help information for a specific command


**DIR**

`cleaner dir .` : List files in the current directory in table format
`cleaner ls .` : List files in the current directory and shows only files in rows below
`cleaner ls "**"` : List files in all subdirectories
`cleaner ls "**" --parents 2` : List files in all subdirectories and prints them with two parent folders

**COPY**

`cleaner copy . /user/username/test/,` : Copy files from the current directory to the specified target directory
`cleaner cp . /user/username/test/, --preview` : Preview copy operation without executing it
`cleaner copy "**" /user/username/test/ --pattern "*.java"` : Copy all Java files from selected directory and subdirectories to the specified target directory and generate directories if not created


**COUNT**

`cleaner count "**"` : Count lines, code lines, code characters, comments and strings in all files in current and subdirectories
`cleaner count "**" --pattern "assert"` : count all assert occurrences in all files in current and subdirectories
`cleaner count "**" --segment comment --pattern "TODO"` : count all TODO occurrences in all files in current and subdirectories within comments
`cleaner count "**" --segment comment --pattern "@PROJECT;@TASK;CODE;TODO"` : count all words between ; in comments

**HISTORY**

`cleaner history -create -local` : Create a local history file in the current directory 

`cleaner find -R --filter "*.h*"  --segment comment --prompt "pattern" --add-to-history "c-pattern"` : Search for pattern that is asked for in comments and also add it to the history under the name "c-pattern"
`cleaner history "c-pattern"` : Run "c-pattern" in history to search for the saved command sequence

`cleaner history -print` : prints the local history file if found
`cleaner history -print -home` : prints the global history for user

`cleaner history -create -home` : Create a global history file for user
`cleaner history -remove --index <index>` : Remove a specific command from history by index
`cleaner history --index <index> --set-alias <alias>` : Set an alias for command and index in history


