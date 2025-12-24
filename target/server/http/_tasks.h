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
