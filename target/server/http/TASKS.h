// @PROJECT [tag: rights, user ] [name: users]
// 

/*

@PROJECT [name: users] [description: Manage user accounts,permissions, login etc ]

@TASK [project: users] [tag: login, cli] [status: open] [user: per]
[summary: Add user login functionality]
[description: Option to webserver to start it with user key, usefull for development and testing]

@TASK [project: users] [tag: table, list] [status: open] [user: per]
[summary: List all users]
[description: Webserver will hold a list of valid users, this also acts as a cache for user information and session logic.]

@TASK [project: users] [tag: endpoint] [status: open] [user: per]
[summary: User endpoints]
[description: Webserver needs endpoints to add, remove or update users. ]

@TASK [project: users] [tag: lookup, verify] [status: open] [user: per]
[summary: Check user for each enpoint called]
[description: When endpoints are called logic to check that the user is valid and has the required permissions.]

  
 */

/*
@PROJECT [name: logging] [tag: log] [description: turn on/off different log types to make development simpler]

@TASK [project: logging] [description: Log SQL to check queries, needed for development} [tag: log, sql] [status: open] [user: per]
*/
 
/*
@PROJECT [name: security, functions] [tag: log] [summary: server methods that are usefull for security]

@TASK [project: security] [description: Get user ip number, may be used to match so that user ip do not change or to generate session for user}
[tag: ip, session] [status: open] [user: per]
*/
 
/*
@PROJECT [name: scripting] [tag: script, lua, python, gdscript] [description: Script logic inside web server]

@TASK [project: scripting] [tag: lua, cli, configuration] [status: open] [user: per]
[summary: initialize lua scripting for webserver]
[description: lua engines are store in a lua pool and this pool has to be initialized at startup]
[sample: http script-pool]
*/
