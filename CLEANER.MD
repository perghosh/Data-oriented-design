# Cleaner information for use

*Cleaner is one tool for organizing and managing files efficiently.*


## Search strings

### key value search

- `cleaner find * --pattern "&c-@TASK" --kv "name;description@[]:" -vs` - *search for `@TASK` in comments*
- `cleaner find * --pattern "&c-@TASK;" --kv "name;description;sample;idea;state;assigned_to@[]:" -vs`

### key value search for per

- `cleaner find * --pattern "&c-@TASK;#user.per" --kv "name;description@[]:" -vs` - *search for `@TASK` in comments*
- `cleaner find * --pattern "&c-@TASK;#user.per" --kv "name;description;sample;idea;state;assigned_to@[]:" -vs`


## Templates

```
@TASK #configuration.load #user.per [name: <name>] 
--
[description: "## " ]

[priority: low|medium|high] [state: open|ongoing|closed|resolved] [assigned_to: <user>]
[idea: "<idea>"]
[sample: "<sample>"]

```