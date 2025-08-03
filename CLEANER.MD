# Cleaner information for use

*Cleaner is one tool for organizing and managing files efficiently.*


## Search strings

### key value search

- `cleaner find * --pattern "&c-@TASK" --kv "name;description@[]:" -vs` - *search for `@TASK` in comments*
- `cleaner find * --pattern "&c-@TASK;" --kv "name;description;sample;idea;state;assigned_to@[]:" -vs`

### key value search for per

- `cleaner find * --pattern "&c-@TASK;#user.per" --kv "name;description@[]:" -vs` - *search for `@TASK` in comments*
- `cleaner find * --pattern "&c-@TASK;#user.per" --kv "name;description;sample;idea;state;assigned_to@[]:" -vs`

### Code sample search

- `cleaner find * --pattern "@code" --segment comment --rule "select-between:@code,@endcode" - *search for `@code` sections in comments and list*

- `cleaner find * --pattern "&c-@code" --rule "select-between:@code,@endcode" - *search for `@TASK` in comments*



## Templates

```
@TASK #<task-area>.<detail> #user.per [name: <name>] 
--
[description: "## <description text>" ]

[priority: low|medium|high] [state: open|ongoing|closed|resolved] [assigned_to: <user>]
[idea: "<idea>"]
[sample: "<sample>"]

```