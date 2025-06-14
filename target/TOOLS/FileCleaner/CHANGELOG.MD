# Changelog


## v 0.9.8

- Fixed relative paths for all commands.
- Argument to match all patterns passed to list command. If three patterns are passed, all three will be matched. Argument is `--match-all` and is a flag.
- Multiple xpattern is possible in `list` command. If multiple patterns are passed, all will be matched.
- rpattern works in string and comment segments. rpattern is a regex patterns.
- context argument where code around the match is printed. Argument is `--context` and you need to pass the first offset line and the number of lines to print around the match. Example: `--context -2,5` will print 2 lines before and 5 lines are extracted.
- Added global modes, based on mode cleaner will behave differently. Mode logic is going to be extended in future releases. Preliminary modes are: `review`, `stats`, `search`, `changes`, `audit`, and `document`.
- string methods for scripting is added. This need to be documented because its advanced usage.
- `dir` command is improved, now it is usefull.
- `dir` have gotten **sort** option to be able to sort the output by name, size, date, or extension. Example: `dir --sort date` will sort by date (default is how old the latest change is in days).
- ``