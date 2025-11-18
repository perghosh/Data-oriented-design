# Documentation: CLEANER_HISTORY.MD XML Structure

## Overview

This XML document is designed to record and organize command history for a file cleaning tool. It tracks saved command entries, including their parameters, aliases, and variable prompts for user interaction.

## Structure

- `<history>`: Root element containing all history data.
  - `<named />`: Reserved for named history entries (currently empty).
  - `<pinned />`: Reserved for pinned history entries (currently empty).
  - `<saved>`: Contains a list of saved command entries.
    - `<entry>`: Represents a single saved command.
      - `<name>`: The type or name of the command (e.g., "find").
      - `<line>`: The command line or parameters used.
      - `<date>`: Timestamp when the entry was saved.
      - `<alias>`: A short alias for the command.
      - `<variable>` (optional): Defines a variable prompt for user input.
        - `ask`: If "1", the tool should prompt the user for this variable.
        - `name`: The variable's name.
        - `description`: Instructions for the user about the variable.

  - `<recent />`: Reserved for recent history entries (currently empty).

## Example Entry

```xml
<entry>
  <name>find</name>
  <line>-R --filter "*.h*" --segment comment --prompt pattern</line>
  <date>2025-10-25 08:46:10</date>
  <alias>c-pattern</alias>
</entry>
```


```xml
<?xml version="1.0"?>
<history>
  <named />
  <pinned />
  <saved>
    <entry>
      <name>find</name>
      <line>-R --filter "*.h*" --segment comment --prompt pattern</line>
      <date>2025-10-25 08:46:10</date>
      <alias>c-pattern</alias>
    </entry>
    <entry>
       <name>find</name>
        <line>"**" --segment comment -clip --pattern "@API" --header "tag;user;title" --brief "summary" --keys "description;desc;detail;code" --kv-where "str::has_tag(tag, '{where}') or str::is_empty('{where}')"</line>
       <date>2025-11-01 11:10:10</date>
       <alias>api</alias>
        <variable ask="1" name="where" description="Enter tag or tags separated with , to select tag. Empty if no filter" />
     </entry>
     <entry>
        <name>find</name>
        <line>"**" --segment comment --pattern "@CODE" --header "tag;user;title" --brief "summary" --keys "description;desc;detail;code" --footer "status;note" --kv-where "str::has_tag(tag, '{where}') or str::is_empty('{where}')"</line>
        <date>2025-11-01 11:10:10</date>
        <alias>code</alias>
        <variable ask="1" name="where" description="Enter tag or tags separated with , to select tag. Empty if no filter" />
     </entry>
     <entry>
        <name>find</name>
        <line>"**" --segment comment --pattern "@FILE" --header "tag;user;title" --brief "summary" --keys "description;desc;detail;code" --footer "status;note" --kv-where "str::has_tag(tag, '{where}') or str::is_empty('{where}')"</line>
        <date>2025-11-01 11:10:10</date>
        <alias>file</alias>
        <variable ask="1" name="where" description="Enter tag or tags separated with , to select tag. Empty if no filter" />
     </entry>
     <entry>
        <name>find</name>
        <line>"**" --segment comment --pattern "@PROJECT" --header "tag;user;title;name" --brief "summary" --keys "description;desc;detail;code" --footer "status;note" --kv-where "name = '{name}' or str::is_empty('{name}')"</line>
        <date>2025-11-01 11:10:10</date>
        <alias>project</alias>
        <variable ask="1" name="name" description="Enter project name to select, empty if no filter finding all projects" />
     </entry>
     <entry>
        <name>find</name>
        <line>"**" --segment comment --pattern "@TASK" --header "project;tle;name" --brief "summary" --keys "description;desc;detail;code" --footer "user;status" --kv-where "(project = '{project}' or str::is_empty('{project}')) and (user = '{user}' or str::is_empty('{user}'))"</line>
        <date>2025-11-01 11:10:10</date>
        <alias>task</alias>
        <variable ask="1" name="project" description="Enter project name to filter tasks for, all project if no filter" />
        <variable ask="1" name="user" description="Enter user name filter tasks for" />
     </entry>
  </saved>
  <recent />
</history>

```