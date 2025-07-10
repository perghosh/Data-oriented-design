### Sample docker commands

- `docker ps` or `docker container ls` - List running docker containers
- `docker ps -a` or `docker container ls -a` - List *all* docker containers
- `docker images` lists images
- `docker run hello-world` - Docker will automatically pull the hello-world image if you don't have it locally.
- `docker run --name my-mariadb -e MARIADB_ROOT_PASSWORD=mysecretpassword -p 3306:3306 -d mariadb`
- `docker stop my-container-name` stops container, this also stop it `docker stop a1b2c3d4e5f6 # (replace with actual container ID)`
- `docker rm container-name` or `docker rm a1b2c3d4e5f6` removes container (stop container before removing it)
- `docker rmi image-name` - to remove docker image
- `docker image prune` - removes all dangling images



### Sample on how to install docker containers

- `docker pull mariadb`
- `docker run --name select-any-name -e MARIADB_ROOT_PASSWORD=mysecretpassword -p 3306:3306 -d mariadb` (`-p` = port mapping, `-d` = detached mode)

### Docker compose



### Sample containers

**mariadb**  
*docker image with mariadb, install and create database with data*

```bash
docker pull mariadb:latest
docker run -d --name mariadb-test -e MARIADB_ROOT_PASSWORD=sa -p 3306:3306 mariadb:latest
docker exec -it mariadb-test mariadb -uroot -psa

CREATE DATABASE test;USE test;
CREATE TABLE tdemo ( demo_k INT AUTO_INCREMENT PRIMARY KEY, f_name VARCHAR(50) NOT NULL );
INSERT INTO tdemo (f_name) VALUES ('john_doe'), ('jane_smith');
SELECT * FROM tdemo;
exit
```
*remove container*
```bash
docker stop mariadb-test
docker rm mariadb-test
docker rmi mariadb
```

**postgres**  
*docker image with postgres, install and create database with data*

```bash
docker pull postgres:latest
docker tag postgres:latest postgres-test
docker run -d --name postgres-test -e POSTGRES_PASSWORD=sa -p 5432:5432 postgres-test
docker exec -it postgres-test psql -U postgres
# Inside the psql prompt, run these commands:
# CREATE DATABASE testdb;
# \c testdb;  (Connect to the new database)
# CREATE TABLE tdemo ( demo_k SERIAL PRIMARY KEY, f_name VARCHAR(50) NOT NULL );
# INSERT INTO tdemo (f_name) VALUES ('john_doe'), ('jane_smith');
# SELECT * FROM tdemo;
# \q (To exit psql)
```

*remove container*
```bash
docker stop postgres-test
docker rm postgres-test
docker rmi postgres-test
```

**python**  
*docker image with python, run some python code*

```bash
docker pull python:latest
docker run -d --name python-test -it python:latest
docker exec -it python-test python

# Example Python code
names = ['john_doe', 'jane_smith']
for i, name in enumerate(names, 1):
    print(f"ID: {i}, Name: {name}")
exit()
```

*remove container*
```bash
docker stop python-test
docker rm python-test
docker rmi python:latest
```
*run image and connect folder*
```bash
docker run -d --name python-test -it -v $(pwd)/code:/code python:latest
```

**alpine with GCC13**
*Using powershell* 
```bash
$currentDir = $PWD.Path
docker run --name alpine-gcc13 -it -v "${currentDir}:/app" -w /app alpine:3.19 /bin/sh -c "apk update && apk add gcc g++ musl-dev make libstdc++-dev && g++ --version && g++ -o cpp docker-cpp.cpp -std=c++20 && ./cpp && /bin/sh"

docker stop alpine-gcc13
docker rm alpine-gcc13
docker rmi alpine:3.19
```