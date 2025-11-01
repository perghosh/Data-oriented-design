```
<?xml version="1.0"?>
<history>
  <named />
  <pinned />
  <saved>
    <entry>
      <name>find</name>
      <line>--filter "*" --pattern @TASK --segment comment --header "project;title" --brief summary --keys description --footer category</line>
      <date>2025-09-13 01:19:40</date>
      <alias>task</alias>
    </entry>
    <entry>
      <name>find</name>
      <line>-R --filter "*.h*" --segment comment --prompt pattern</line>
      <date>2025-10-25 08:46:10</date>
      <alias>c-pattern</alias>
    </entry>
    <entry>
      <name>find</name>
      <line>-R "**" --segment comment --pattern {pattern}</line>
      <date>2025-10-25 08:46:10</date>
      <alias>api</alias>
      <variable name="pattern" description="The pattern to search for" />
    </entry>
  </saved>
  <recent />
</history>
```


`"args": [ "history", "--run", "api" ],`