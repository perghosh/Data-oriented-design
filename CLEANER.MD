# Cleaner information for use

*Cleaner is one tool for organizing and managing files efficiently.*


## Search strings

### key value search

- `cleaner find * --pattern "&c-@TASK" --kv "name;description@[]:" -vs` - *search for `@TASK` in comments*
- `cleaner find * --pattern "&c-@TASK;" --kv "name;description;sample;idea;state;owner@[]:" -vs`

### key value search for per

- `cleaner find * --pattern "&c-@TASK;#user.per" --kv "name;description@[]:" -vs` - *search for `@TASK` in comments*
- `cleaner find * --pattern "&c-@TASK;#user.per" --kv "name;description;sample;idea;state;owner@[]:" -vs`

### Code sample search

- `cleaner find * --pattern "@code" --segment comment --rule "select-between:@code,@endcode" - *search for `@code` sections in comments and list*

- `cleaner find * --pattern "&c-@code" --rule "select-between:@code,@endcode" - *search for `@TASK` in comments*


****
## Templates

```
@TASK #<task-area>.<detail> #user.per [name: <name>] 
--
[description: "## <description text>" ]

[priority: low|medium|high] [state: open|ongoing|closed|resolved] [owner: <user>]
[idea: "<idea>"]
[sample: "<sample>"]

```

```
@TASK #<task-area>.<detail> #user.per [name: <name>] [description: "## <description text>" ]
```


*   **`@API`**: Describes code sections and endpoints.
*   **`@FILE`**: Describes the file (always placed at the top).
*   **`@PROJECT`** & **`@TASK`**: For project management. Searching for a project name lists all its tasks.
*   **`@TODO`**: For simple to-do items.
*   **`@SAMPLE`**: For example code snippets.
*   **`@CLASS`**: classes and structs
*   **`@DEBUG`**: code used for debug  