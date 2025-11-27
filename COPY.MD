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
        <line>"**" --segment comment --pattern "@API" --header "tag;user" --keys "description;desc;detail;code" --footer "status;note" --kv-where "str::has_tag(tag, '{where}') or str::is_empty('{where}')"</line>
       <date>2025-11-01 11:10:10</date>
       <alias>api</alias>
        <variable ask="1" name="where" description="Enter tag or tags separated with , to select tag. Empty if no filter" />
     </entry>
     <entry>
        <name>find</name>
        <line>"**" --segment comment --pattern "@CODE" --header "tag;user" --keys "description;desc;detail;code" --footer "status;note" --kv-where "str::has_tag(tag, '{where}') or str::is_empty('{where}')"</line>
        <date>2025-11-01 11:10:10</date>
        <alias>code</alias>
        <variable ask="1" name="where" description="Enter tag or tags separated with , to select tag. Empty if no filter" />
     </entry>
  </saved>
  <recent />
</history>

```


`"args": [ "history", "--run", "api" ],`