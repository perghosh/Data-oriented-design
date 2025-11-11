/*
Searches for tasks in the file and lists them here.
cleaner find -filter "*.h;*.cpp" -R --pattern "@TASK" --segment comment --keys "description;sample;idea" --header "project;title;id" --brief "summary" --footer "created;assignee;status"  --prompt "kv-where"

*/

/*
@PROJECT [name: hot] [description: Hot operations, collec what to do here]

@TASK [title: prompt] [project:hot] [assignee: per] [status:open] [level: 2] [created: 250830]
[summary: running prompt values from history do not ask for value because it is done later compared to original]
[description: "Move logick for prompt to enable set prompt value from history"]

@TASK [title: prompt] [project:hot] [assignee: per] [status:open] [level: 1] [created: 250830]
[summary: better description for prompt values]
[description: "Add better description for prompt values, what it does and how to use it"]
*/


/*
@PROJECT [name: prompt]

@TASK [title: add prompt option to options class] [project:prompt] [assignee: per] [status:open] [created: 250828]
[summary : Add prompt option to options class]
[description: "This task involves adding a prompt option to the options class, allowing users to specify whether they want to be prompted for input before executing a command."]

@TASK [title: add prompt values] [project:prompt] [assignee: per] [status:open] [created: 250828]
[summary : Add values specified in prompt]
[description: "Prompt values are separated with ; if found the add those to options before reading values from arguments"]

*/

// ----------------------------------------------------------------------------

/*
@PROJECT [name: history] [description: History operations]

@TASK [title: home directory] [project:history] [assignee: kevin] [status:closed] [created: 250830]
[summary : Replace GetHistoryPath_s with CApplication::FolderGetHome_s to get home directory]

@TASK [title: pin history item] [project:history] [assignee: per] [status:open] [created: 250828]
[summary : Pin a history item]
[description: "This task involves adding a pin feature to the history, allowing users to pin specific commands for easy access later."]   

*/

// ----------------------------------------------------------------------------

/*
@PROJECT [name: print]

@TASK [title: check that csv print works] [project:print] [assignee: per] [status:open] [created: 250828]
[summary: Check that CSV print works]
[description: "This task involves verifying that the CSV print functionality is working as intended."]

@TASK [title: border/frame] [project:print] [assignee: per] [status:open] [created: 250828]
[summary: argument to set border/frame for output row]
[description: "Frame is used if configured doing find searches with key value, with -frame or similar a frame should be printed even if no header or footer is set."]

@TASK [title: format template] [project:print] [assignee: per] [status:open] [level:4] [created: 250830]
[summary: template for printing]
[description: "To get better way to format output based on what type of search thats done figure out some sort of template to read information from. Similar to jinja maybe"]

*/

// ----------------------------------------------------------------------------

/*
@PROJECT [name: help]

@TASK [summary: add global options to command help] [project:help] [assignee: per] [status:closed] [created: 251111]
[summary : print help for specific command, then global options should be printed also]

@TASK [summary: print only global options] [project:help] [assignee: per] [status:closed] [created: 251111]
[summary : add flag for only print global options]
*/

// ----------------------------------------------------------------------------

/*
@PROJECT [name: serialize-table] [summary: Serialize and deserialize table data]

*/
