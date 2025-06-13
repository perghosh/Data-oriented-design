# Changelog


## v 0.9.8

- Fixed relative paths for all commands.
- Argument to match all patterns passed to list command. If three patterns are passed, all three will be matched. Argument is `--match-all` and is a flag.
- Support for `--match-all` in `list` command. All patterns passed to the command will be matched.
- Multiple xpattern is possible in `list` command. If multiple patterns are passed, all will be matched.
- xpattern works in string and comment segments.
- context argument where code around the match is printed. Argument is `--context` and you need to pass the first offset line and the number of lines to print around the match. Example: `--context -2,5` will print 2 lines before and 5 lines are extracted.
- Added global modes, based on mode cleaner will behave differently. Mode logic is going to be extended in future releases. Preliminary modes are: `review`, `stats`, `search`, `changes`, `audit`, and `document`.
- string methods for scripting is added. This need to be documented because its advanced usage.