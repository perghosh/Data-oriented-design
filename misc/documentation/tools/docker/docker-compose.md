## Docker Compose Commands, sample usage for frontend and backend projects using python

*Sample that shows a scenario where you have one backend and one frontend in same repo*

### Basic Service Control:
- `docker-compose up backend` - *Start only the backend*
- `docker-compose up -d backend` - *Start backend in detached mode (background)*
- `docker-compose stop backend` - *Stop only the backend*
- `docker-compose restart backend` - *Restart only the backend*

### Most Efficient Workflow:

1. **First time setup** (starts everything):
   - `docker-compose up -d` - *Start all services in background*

2. **Daily development** - if you only need to restart backend:
   - `docker-compose restart backend` - *Quick restart after code changes*

3. **If you want to see backend logs while developing:**
   - `docker-compose up backend` - *Start backend with live logs*

### Other Useful Commands:

- `docker-compose ps` - *Check what's running*
- `docker-compose logs -f backend` - *View logs for specific service*
- `docker-compose up -d db backend` - *Start multiple specific services*
- `docker-compose down` - *Stop everything*

### For Your Typical Development Workflow:

Since your backend has `restart: always`, if you just need to restart it after code changes:
- `docker-compose restart backend` - *Fastest option - restarts existing container without recreating it*

This is much faster than `docker-compose up` because it doesn't recreate containers or run the prestart script again - it just restarts the existing backend container while keeping database, mailcatcher, and frontend running in the background.

### Full Clean Rebuild (Recommended):
- `docker-compose down && docker-compose build --no-cache && docker-compose up -d` - *Stop, rebuild without cache, and start*


### Nuclear Option (Complete Clean Slate):
- `docker-compose down --rmi all --volumes --remove-orphans` - *Remove everything including volumes*
- `docker-compose build --no-cache` - *Rebuild from scratch*
- `docker-compose up -d` - *Start fresh*

### Alternative Docker System Commands:
- `docker system prune -a` - *Remove all unused containers, images, and networks*
- `docker builder prune` - *Clear build cache*
- `docker volume prune` - *Remove unused volumes (⚠️ will delete your database data)*

### Quick Rebuild (Less Aggressive):
- `docker-compose build --no-cache backend` - *Rebuild only backend service*
- `docker-compose up -d --force-recreate backend` - *Force recreate backend container*

**⚠️ Warning:** Using `--volumes` will delete your PostgreSQL data. Only use it if you want to completely start over with an empty database.


