import docker
import time
import mysql.connector
from mysql.connector import Error

def wait_for_mariadb(host, port, user, password, max_retries=30, delay=2):
    """Wait for MariaDB to be ready to accept connections"""
    for attempt in range(max_retries):
        try:
            conn = mysql.connector.connect(host=host, port=port, user=user, password=password, charset='utf8mb4', collation='utf8mb4_general_ci')
            conn.close()
            return True
        except Error as e:
            print(f"Attempt {attempt + 1}/{max_retries}: MariaDB not ready yet... ({e})")
            time.sleep(delay)
    return False

def main():
    # Step 1: Pull the mariadb image
    client = docker.from_env()
    print("Pulling mariadb:latest image...")
    client.images.pull('mariadb:latest')
    
    # Step 2: Run the container
    print("Starting mariadb-test container...")
    try:
        # Remove existing container if it exists
        try:
            existing_container = client.containers.get('mariadb-test')
            existing_container.stop()
            existing_container.remove()
            print("Removed existing container")
        except docker.errors.NotFound:
            pass
        
        container = client.containers.run('mariadb:latest', name='mariadb-test', environment={'MARIADB_ROOT_PASSWORD': 'sa'}, ports={'3306/tcp': 3306}, detach=True)
        
        # Step 3: Wait for MariaDB to initialize
        print("Waiting for MariaDB to initialize...")
        if not wait_for_mariadb('127.0.0.1', 3306, 'root', 'sa'):
            print("MariaDB failed to start within the timeout period")
            return
        
        # Step 4: Connect to MariaDB and execute SQL commands
        print("Connecting to MariaDB and executing SQL commands...")
        conn = mysql.connector.connect(host='127.0.0.1', port=3306, user='root', password='sa', charset='utf8mb4', collation='utf8mb4_general_ci')
        
        cursor = conn.cursor()
        
        # Create database and table
        cursor.execute("CREATE DATABASE IF NOT EXISTS test;")
        cursor.execute("USE test;")
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS tdemo (
                demo_k INT AUTO_INCREMENT PRIMARY KEY,
                f_name VARCHAR(50) NOT NULL
            );
        """)
        
        # Insert data
        cursor.execute("INSERT INTO tdemo (f_name) VALUES ('john_doe'), ('jane_smith');")
        conn.commit()  # Commit the transaction
        
        # Query data
        cursor.execute("SELECT * FROM tdemo;")
        print("Query results:")
        for row in cursor.fetchall():
            print(row)
        
        cursor.close()
        conn.close()
        
        print("\nDone. To stop and remove the container, run:")
        print("docker stop mariadb-test && docker rm mariadb-test")
        
    except Error as e:
        print(f"Database error: {e}")
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    main()