// @PROJECT [tag: rights, user ] [name: users]
// 

/* ## USERS

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

/* ## LOGGING
@PROJECT [name: logging] [tag: log] [description: turn on/off different log types to make development simpler]

@TASK [project: logging] [description: Log SQL to check queries, needed for development} [tag: log, sql] [status: open] [user: per]
*/
 
/* ## SECURITY
@PROJECT [name: security, functions] [tag: log] [summary: server methods that are usefull for security]

@TASK [project: security] [description: Get user ip number, may be used to match so that user ip do not change or to generate session for user]
[tag: ip, session] [status: open] [user: per]

@TASK [project: security] [description: Max size for post data for statement. This to avoid specific problems for selected statement] 
[tag: post, size] [status: open] [user: per]
*/
 
/* ## SCRIPTING
@PROJECT [name: scripting] [tag: script, lua, python, gdscript] [description: Script logic inside web server]

@TASK [project: scripting] [tag: lua, ssr] [status: open] [user: per]
[summary: access variables in lua for ssr]
[description: running lua scripts in server-side rendering that are able to access and manipulate variables]

@TASK [project: scripting] [tag: lua, ssr, session] [status: open] [user: per]
[summary: access session in lua for ssr]
[description: running lua scripts in server-side rendering that are able to access and manipulate session data]


*/


/* ## multiple database command packed in xml
@PROJECT [name: command-packing] [tag: xml] [description: Pack commands in xml]

@TASK [project: command-packing] [tag: xml] [status: open] [user: per]
[summary: multiple endoints inside xml]
[description: add js method that takes raw string for endpoint and then object that is converted to string that mimics arguments]

@TASK [project: command-packing] [tag: xml] [status: open] [user: per]
[summary: multiple endoints inside xml]
[description: run lua scripts in server-side rendering context]




@TASK [project: command-packing] [tag: xml] [status: open] [user: per]
[summary: delete poll and related data]
[description: delete poll record and its related data using command packing]


*/
