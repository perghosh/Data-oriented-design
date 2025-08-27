# Changelog

## v1.0.5
### New Features
- Added support for multi-threaded operations to improve performance and responsiveness.
- Introduced a new `--parallel` option to the `find` command to enable parallel processing of files.
- All `count` searches are now performed in parallel, significantly speeding up the counting process for large codebases.
- Local `config` option argument added to specify a configuration file to use for the cleaner. This allows you to have different configurations for different projects or tasks.
- New `paste` command that reads clipboard content and pastes it into the cleaner. This allows you to quickly add content from the clipboard to the cleaner without having to type it in manually.
- `list` command is parallelized, which means that it can now process multiple files at the same time, improving performance for large codebases.
- `kv-where` filter for key value pairs added to the `find` command. This allows you to filter results based on specific key value pairs, making it easier to find relevant information.
- `kv-where` tries to figure out in what format condition is written in and converts it internally to the right one
- header and footer values that is not found will not add , character
- `kv-format` added fix to remove format by settng "!" as format. This allows you to read key value without format even if format is set in configuration file.
- `prompt` is a new global option and its huge, this enables to specify arguments inside cleaner, cleaner asks for option values specified with prompt and with that you do not need to write that much. Easier to reuse same command line options

## v1.0.3
### New Features
- Global `config` option argument added to specify a configuration file to use.
- Global `logging-severity` to specify the logging level for cleaner. This allows you to control the verbosity of the output.
- Extended configuration and make it possible to select different configuration files. Added more settings to configuration like ignore and logging settings.
- `--footer` added to find command to specify a footer for the output. This allows you to add custom text at the end of the output.
- `--brief` added to find command to enable brief output mode. This to enable reducing the amount of output and focus on the most important information.
- Global `--history` option to save the last used command to a history file. This allows you to easily repeat the last command without having to type it again.
- `--width` Added to find command to specify the width of the output. This allows you to control how wide the output is.
- More colors and settings added to the configuration file to allow for more customization of the cleaner behavior. Things like how to draw headers, footers, and brief output.
    - sample "cleaner.format": { "kv": "[]:", "header-line": "0xDAC4BF", "footer-line": "0xC0C4D9", "brief": ">>> ", "width": 100 }
    - "cleaner.logging": { "severity": null },
    - "cleaner.ignore": {"folder": null, "file": null }
 - Added colors to `count` result, for now only one color
 - Sort works on `count` tables.

## v1.0.2

Enhanced results formatting to improve clarity and make the output more user-friendly and visually appealing.
Enable configuration management for cleaner, user-defined settings for cleaner operations to make it easier to customize the cleaner behavior for personal preferences.

### New Features
- `header` option added to `find` command. This allows you to specify a header for the output of the find command. The header will be printed before the line result printing key value pairs.
- Preview of result lines when clickable rows are trimed to simplify what is shown in the result.
- `config` command added to manage cleaner configuration files. This allows you to create, edit, and delete configuration files for cleaner.
- `config -create` creates a new configuration file and places it in user home directory under `~/.cleaner/`.****
- `config -edit` opens confguration file in**** the default editor for easy editing. Its a `json` file.
- file with themes added to zipped cleaner package that can be downloaded among releases. This allows you to easily change the look and feel of the cleaner.
- Generated results are colored based on the theme selected in the configuration file. This allows you to customize the look of the cleaner.
- Color codes for 256 color mode or true RGB color can be used, what to select is up to you and the support terminal have for colors.
- Output for key value pairs is now formated better, with each key value pair on a new line and indented for better readability.

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
  - `segment` added to `dir` command. Not fully implemented yet, but allows you to search for patterns in specific segments of the code, such as comments or strings.


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