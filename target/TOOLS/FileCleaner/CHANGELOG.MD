# Changelog

## v1.0.1
### New Features

Added support for **key value** reading. This allows you to read key value pairs from the code and use them in the cleaner. Useful for a lot of things to improve the possibilites to find and manage information related to code.
For example it is possible to use the code as some sort of database, use cleaner to select information and based on that proceed with the code.
Things like tasks, features, bugs etc. Also bookmarks with descriptions and much more.

- `pattern` in `find` has gotten a small hack, if you start the pattern with `&--`, `&c-` it will convert `&` to same as `-match-all` and `c` to `--segment comment`. This hack allows you to quickly set options without too much typing. If you work with key value pairs this is very useful.
- `keys` option added to `find` command. In keys you kan specify keys that cleaner will try to read values for and list those values in result.
- `kv-format` option added to `find` command. This allows you to specify how the key value pairs are formatted in code. 
  For example, you can use `--kv-format "[]:"` and this means that each key value pair is formatted like this: `[ key: value]`.
  - `kv` option added to `find` command. This is a shortcut for both `keys` and `kv-format` options. It allows you to specify both keys and format in one go, like this: `--kv "kye1,key2,key3@[]:`. Trickier but to make it fast to type.
  - Improved help output do describe options used by cleaner better.
  - Sumarized output when printing result to output.


## v1.0.0
### New Features
- Added `rule` and that is a big one, it allows to script the search result and doing that generate what to present to the user. Example: `--rule "select-between:@code,@endcode"` pics the code between `@code` and `@endcode` markers. Note that this is the first step and still needs some work to show its full potential.
- Introduced `verbose` flag for all commands to enable detailed output. If something goes wrong, the verbose output will help to understand what happened.
- `segment` added to find command. Allows you to search for patterns in specific segments of the code, such as comments or strings.
- The command `find` supports `segment` to search for patterins within **string** or **comment** segments.
- Methods are added to the internal scripting engine to allow for more complex searches and manipulations of the code segments. But it's not yet possible to write scripts that can be used in the cleaner.
- Configuration for `zig` added to handle zig files.

## v0.9.9

### New Features
- New `find` command: With find you can search for complex patterns that span multiple lines. Before it searches code is stripped of comments and strings.
  - `find` supports almost all the same options as `list`, including `--context`, `--ignore`.
  - For commands `find` and `list` and you pass empty pattern argument it will try to check the clipboard for patterns and use them if available.

### Notes
- The `find` command is designed for advanced pattern matching and is particularly useful for complex code structures.
- Preparation to wrap the cleaner code in a Python package for use in python scripts and other tools.

## v0.9.8

### Fixes
- Fixed relative path handling for all commands.

### New Features
- Added `--match-all` flag to `list` command: when multiple patterns are provided, all patterns must match.
- Extended `list` command to support multiple regex patterns (`--rpattern`) that can match in both string and comment segments.
- Introduced `--context` option to display surrounding code for matches. Specify line offset and range (e.g., `--context -2,5` shows 2 lines before and 5 lines after the match).
- Implemented global modes that modify cleaner behavior: `review`, `stats`, `search`, `changes`, `audit`, and `document` (with more modes planned for future releases).
- Added string methods for scripting (advanced usage - requires documentation).

### Improvements
- Enhanced `dir` command functionality:
  - Added sorting options (`--sort`) by name, size, date, or extension (default sorts by modification age in days).
- Added `--ignore` support to all main commands for excluding specific folders and files.
- Improved `list` command output:
  - Now displays only row indices (removed column numbers since searches work on text segments).
  - Shows complete lines for matches rather than partial line segments.

### Notes
- The mode logic will be expanded in future updates.
- Documentation is needed for the new scripting string methods.

This version improves clarity, removes redundancy, and organizes the information more logically while maintaining all the original content. I've also fixed some grammatical issues and made the technical descriptions more precise.