## URLs to test HTTP server
- `http://localhost:8001/!db/create?type=sqlite&name=test` Create a new SQLite database named "test".
- `http://localhost:8001/!db/open?type=sqlite&name=test` Open SQLite database named "test".


# Define variables
$site = "http://localhost:8001"

`curl "${site}/!db/open?type=sqlite&name=test"

--data-urlencode "page=1"

`curl "${site}/!db/open/query?type=sqlite&name=test&query=CREATE TABLE TUser( UserK BLOB PRIMARY KEY DEFAULT (randomblob(16)), FName TEXT NOT NULL, FAlias TEXT NOT NULL);"`

curl "${site}/!db/open/query?type=sqlite&name=test" --data-urlencode "query=CREATE TABLE TUser( UserK BLOB PRIMARY KEY DEFAULT (randomblob(16)), FName TEXT NOT NULL, FAlias TEXT NOT NULL);" -G



--data-urlencode "query=CREATE TABLE TUser( UserK BLOB PRIMARY KEY DEFAULT (randomblob(16)), FName TEXT NOT NULL, FAlias TEXT NOT NULL);"