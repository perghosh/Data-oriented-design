/*
Searches for tasks in the file and lists them here.
cleaner find -filter "*.h;*.cpp" -R --pattern "@TASK" --segment comment --keys "description;sample;idea" --header "project;title;id" --brief "summary" --footer "created;user;status"  --prompt "kv-where"

*/

/*
@PROJECT [name: hot] [description: Hot operations, collec what to do here]

@TASK [title: prompt] [project:hot] [user: per] [status:open] [level: 2] [created: 250830]
[summary: running prompt values from history do not ask for value because it is done later compared to original]
[description: "Move logick for prompt to enable set prompt value from history"]

@TASK [title: prompt] [project:hot] [user: per] [status:open] [level: 1] [created: 250830]
[summary: better description for prompt values]
[description: "Add better description for prompt values, what it does and how to use it"]
*/


/*
@PROJECT [name: prompt]

@TASK [title: add prompt option to options class] [project:prompt] [user: per] [status:open] [created: 250828]
[summary : Add prompt option to options class]
[description: "This task involves adding a prompt option to the options class, allowing users to specify whether they want to be prompted for input before executing a command."]

@TASK [title: add prompt values] [project:prompt] [user: per] [status:open] [created: 250828]
[summary : Add values specified in prompt]
[description: "Prompt values are separated with ; if found the add those to options before reading values from arguments"]

*/

// ----------------------------------------------------------------------------

/*
@PROJECT [name: history] [description: History operations]

@TASK [title: home directory] [project:history] [user: kevin] [status:closed] [created: 250830]
[summary : Replace GetHistoryPath_s with CApplication::FolderGetHome_s to get home directory]

@TASK [title: pin history item] [project:history] [user: per] [status:open] [created: 250828]
[summary : Pin a history item]
[description: "This task involves adding a pin feature to the history, allowing users to pin specific commands for easy access later."]

@TASK [title: unpin history item] [project:history] [user: per] [status:open] [created: 250828]
[summary : Unpin a history item]
[description: "This task involves adding an unpin feature to the history, allowing users to unpin specific commands that are no longer needed."]

@TASK [title: check for duplicate aliases] [project:history] [user: per] [status:open] [created: 251111]
[summary : Check for duplicate aliases]
[description: "This task involves checking for duplicate aliases in the history. If there are any print some sort of warning"]

*/

// ----------------------------------------------------------------------------

/*
@PROJECT [name: print]

@TASK [title: check that csv print works] [project:print] [user: per] [status:open] [created: 250828]
[summary: Check that CSV print works]
[description: "This task involves verifying that the CSV print functionality is working as intended."]

@TASK [title: border/frame] [project:print] [user: per] [status:open] [created: 250828]
[summary: argument to set border/frame for output row]
[description: "Frame is used if configured doing find searches with key value, with -frame or similar a frame should be printed even if no header or footer is set."]

@TASK [title: format template] [project:print] [user: per] [status:open] [level:4] [created: 250830]
[summary: template for printing]
[description: "To get better way to format output based on what type of search thats done figure out some sort of template to read information from. Similar to jinja maybe"]

*/


// ----------------------------------------------------------------------------

/*
@PROJECT [name: many-command-lines] [summary: Pass many command lines, in file or clipboard]

@TASK [summary: add subcommand] [project:many-command-lines] [user: per] [status:open] [created: 251111] [type: code]
[description: add subbcommand to cleaner that take a file or checks for clipboard where it should be able to process multiple lines and execute them]

@TASK [summary: help for subcommand] [project:many-command-lines] [user: per] [status:open] [created: 251111] [type: code]
[description: fix help for subcommand]

@TASK [summary: write tests for cleaner] [project:many-command-lines] [user: per] [status:open] [created: 251111] [type: code]
[description: this many command line should be used to test cleaner for proper functionality]

@TASK [summary: docs for many-command-lines] [project:many-command-lines] [user: per] [status:open] [created: 251111] [type: docs]
[description: add documentation for many-command-lines logic]

@TASK [summary: test many-command-lines] [project:many-command-lines] [user: per] [status:open] [created: 251111] [type: test]
[description: test many-command-lines so it works]

*/

/*
@PROJECT [name: folder operations] [summary: Perform operations on folders] [priority: high]

@TASK [summary: sum count based on folder] [project:folder operations] [user: per] [status:open] [created: 251112] [type: code]
[description: count information in files is normally presented for each file, summing up the total count for the folder is also needed]

*/

/*
@PROJECT [name: overload] [summary: Add overloading of arguments using the history command]

@TASK [summary: overload history command arguments] [project:overload] [user: per] [status:open] [created: 251114] [type: code]
[description: Running history commands reads arguments from saved history string, but to change check what type of arguments that is passed running the history command]

@TASK [summary: test that overload works] [project:overload] [user: per] [status:open] [created: 251114] [type: code]
[description: Check to see how the overload works, there might be problems there last time I checked]

*/

/*
@PROJECT [name: ignore] [summary: Global flag to disable ignore settings]

@TASK [summary: flag to not read ignore file when these need to be read] [project:ignore] [user: per] [status:closed] [created: 251126] [type: code]
[description: Sometimes you need to find information within files that are ignored, add flag (global) to disable ignore flags]
*/

/*
@PROJECT [name: history] [summary: add menu and list to history] [priority: high]

@TASK [summary: menu shows aliases with numbers to select] [project:history] [user: per] [status:closed] [created: 251126] [type: code]
[description: menu should list aliases with numbers to be selected, also comment if it exists]
*/


/*
@PROJECT [name: command-find-tags] [summary: print all tags for selected key]

@TASK [summary: command to find all tags for selected key] [project:command-find-tags] [user: per] [status:open] [created: 251130] [type: code]
[description: Add command to find all tags for selected key, similar to find keys but only print tags for those keys. This is important to see what tags that are selectable]

*/

/*
@PROJECT [name: folder-aliases-in-config] [summary: set folder aliases in config to select where to search]
[description: To simplify on how to configure and where to search in larger projects adding aliases for folders
and beeing able to use these when searching in different projects might help]

@TASK [project: folder-aliases-in-config] [user: per] [status:open] [created: 251226] [type: howto]
[description: Try to figure out a simple way to use aliases in with cleaner when searching, lets say
that you are in root folder and from there it should be easy to just use some alias for folder
and work in that subfolder. But whats problematic is that if you need to do this all the time.]


@TASK [project: folder-aliases-in-config] [user: per] [status:open] [created: 251226] [type: code]
[description: In configuration file add aliases for folders to be able to select where to search]

@TASK [project: folder-aliases-in-config] [user: per] [status:open] [created: 251226] [type: code]
[description: Have a file with latest settings or active]

*/
