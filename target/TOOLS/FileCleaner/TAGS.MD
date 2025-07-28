## Sample format for tags in code

`cleaner list * --segment comment  -pattern "@TAG;#ui;#command.cli" -match-all` - row matching, tries to find all tags inside comments in code files.
`cleaner find * --segment comment  -pattern "@TAG;#command.cli" -match-all` - source code search, tries to find all tags inside comments in code files. find treats source code as a single block of text.

### Main section tags

| Tag         | Description                |
|-|-|
| @TAG        | Main tag to mark places where to navigate to places in code based on functionality  |
| @TASK       | Main tag to mark things to work on |

### Sub tags used to filter tag searches

| Search tag | Description |
|-|-|
| #user | tag for mark user to find functionality. `#user.<subtag>` to mark user. <br>Adding user name will make it simple for user to search for like `#user.per` or `#user.kevin` |
| #ui | tag for user interface related logic, some subtags are `#ui.cli` for terminal, `#ui.vs` for visual studio  |
| #data | tag for data related logic, some subtags are `#data.cache` for caching logic, `#data.storage` for storage logic etc |
| #command | tag for command related logic, subtags to see what type of command. Sample `#command.history` for history command |
| #options | tag for options related logic, subtags to see what type of options. Sample `#options.verbose` for verbose option |
| #docs | tag for documentation. `#docs.sample` for sample code, `#docs.primer` for introduction.  |
| #hack | solutions that are like hacks, they do not follow best practices or the core logic and need special handling |
| #config | tag for configuration related logic.<br>`#config.<subtag>` as `#config.docker` for docker functionality |
