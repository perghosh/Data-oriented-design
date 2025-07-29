## Sample format for tags in code

`cleaner list * --segment comment  -pattern "@TAG;#ui;#command.cli" -match-all` - row matching, tries to find all tags inside comments in code files.
`cleaner find * --segment comment  -pattern "@TAG;#command.cli" -match-all` - source code search, tries to find all tags inside comments in code files. find treats source code as a single block of text.
`cleaner find --filter "*" -R --segment comment --pattern "@TODO"  --keys "name;description;idea;state" --kv-format "[]:"` - find @TODO in comments and extract name,description,idea,state values

### Main section tags

| Tag         | Description                |
|-|-|
| @TAG        | Main tag to mark places where to navigate to places in code based on functionality  |
| @TODO       | Things that need to be done |
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


### Samples 

*Sample tags and what to add to improve usability*
```
@TODO #user.per # [name: ] [description: describe that to do] [ideas: add ideas to improve this task] [priority: high, medium, low] [status: open, in-progress, closed]
- name: name for todo task
- description: description of the task
- ideas: ideas to improve implementation of the task
- priority: priority of the task, high, medium, low
- status: status of the task, open, in-progress, blocked, closed, done

@TASK #user.per #data.cache [name: ] [system: system area in code ] [description: describe that task] [ideas: add ideas to improve this task] [priority: high, medium, low] [status: open, in-progress, closed]
- name: name for task
- description: description of the task
- ideas: ideas to improve implementation of the task
- priority: priority of the task, high, medium, low
- status: status of the task, open, in-progress, closed
```

### Key tags

*Keys that might be usefull for tasks, todos and similar tags*

| Key | Description |
|-|-|
| id | Unique identifier for the task |
| name | Name of the task or as within the sample, name of TODO item |
| description | Detailed description of the task |
| ideas | Ideas for improving the task |
| priority | Priority level of the task (high, medium, low) |
| status | Current status of the task (open, in-progress, blocked, closed, done) |
| system | System area in code where the task is relevant |
| created_by | User who created the task |
| created_at | Timestamp when the task was created |
| updated_by | User who last updated the task |
| assigned_to | User to whom the task is assigned |
| link | External link to a related issue, documentation, or design document (e.g., Jira ticket, Confluence page). |
| context | A brief description of the code area or component this task relates to, as well as why it is important. |
| time-spent | Time spent on the task (e.g., in hours). |

*task sample*

```cpp
/*
@TODO #user.per [id: T1234]
[name: help]
[description: """
  The current help output is hard to read.
  It needs line wrapping, consistent indentation,
  and a configurable line width for better readability in terminals.
  """]
[ideas: """
  - Implement a text wrapping utility function.
  - Add a callback mechanism for custom formatting of output blocks.
  - Allow configuration via a `cli_options.json` file.
  """ ]
[priority: high] [status: open] [assigned_to: per]
*/
```


*Tutorial related key tags*

| Key | Description |
|-|-|
| simplicity | simple, medium, hard |
| example | Tag for marking example code snippets. |
| explanation | Tag for marking explanations of code or concepts. |
| sample | Tag for marking sample code or configurations. |
