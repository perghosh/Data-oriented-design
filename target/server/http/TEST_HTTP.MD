## URLs to test HTTP server
- `http://localhost:8001/!db/create?type=sqlite&name=test` Create a new SQLite database named "test".
- `http://localhost:8001/!db/open?type=sqlite&name=test` Open SQLite database named "test".


# Define variables
$site = "http://localhost:8000"
$site = "http://localhost:8001"

**create**
curl "${site}/!db/open/query?type=sqlite&name=test" --data-urlencode "query=CREATE TABLE TUser( UserK BLOB PRIMARY KEY DEFAULT (randomblob(16)), FName TEXT NOT NULL, FAlias TEXT NOT NULL);" -G

`curl "${site}/!db/open?type=sqlite&name=test"

--data-urlencode "page=1"

`curl "${site}/!db/open/query?type=sqlite&name=test&query=CREATE TABLE TUser( UserK BLOB PRIMARY KEY DEFAULT (randomblob(16)), FName TEXT NOT NULL, FAlias TEXT NOT NULL);"`

curl "${site}/!db/open/query?type=sqlite&name=test" --data-urlencode "query=CREATE TABLE TUser( UserK BLOB PRIMARY KEY DEFAULT (randomblob(16)), FName TEXT NOT NULL, FAlias TEXT NOT NULL);" -G



--data-urlencode "query=CREATE TABLE TUser( UserK BLOB PRIMARY KEY DEFAULT (randomblob(16)), FName TEXT NOT NULL, FAlias TEXT NOT NULL);"

**insert user**
curl "${site}/!db/open/insert?type=sqlite&name=test" --data-urlencode "query=INSERT INTO TUser (FName, FAlias) VALUES ('John Doe', 'jdoe');" -G

**insert with select**
"INSERT INTO TUser (FName, FAlias) VALUES ('test1', 'test1') RETURNING UserK;";
curl "${site}/!db/open/query?type=sqlite&name=test" --data-urlencode "query=INSERT INTO TUser (FName, FAlias) VALUES ('test1', 'test1') RETURNING UserK;" -G


SELECT name FROM sqlite_master WHERE type='table'


curl "${site}/!db/open/select?type=sqlite&name=test" --data-urlencode "query=SELECT name FROM sqlite_master WHERE type='table';" -G

curl "${site}/!db/open/select?type=sqlite&name=test" --data-urlencode "query=SELECT * FROM TUser;" -G

$site = "http://localhost:8001"
curl "${site}/!db/open/select?type=sqlite&name=test" --data-urlencode "query=SELECT * FROM TUser;" -G
curl "${site}/!db/open/select/select?type=sqlite&name=test" --data-urlencode "query=SELECT * FROM TUser" --data-urlencode "query=SELECT FName,FAlias FROM TUser" -G
curl "${site}/!db/open/select?type=sqlite&name=test" --data-urlencode "query=SELECT * FROM TUserRights;" -G

open/select?type=sqlite&name=test&query=SELECT * FROM TUser

open/select/select?type=sqlite&name=test&query=SELECT * FROM TUser&query=SELECT FName,FAlias FROM TUser
