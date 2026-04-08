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
@PROJECT [name: vote-detail] [tag: html] [description: test logic for adding vote information for vote]

@TASK [project: vote-detail] [tag: vote, questions] [status: open] [user: per]
[description: Add questions to vote, this is the information about the vote that is shown to user when they click on a vote in the list of quetions.]

@TASK [project: vote-detail] [tag: vote, answer] [status: open] [user: per]
[description: Add answer information to questions, what type of answers that ar valid for vote]

@TASK [project: vote-detail] [tag: vote, rules] [status: open] [user: per]
[description: Add rules for vote, this is the logic that decide what is possible for voter]


 */
