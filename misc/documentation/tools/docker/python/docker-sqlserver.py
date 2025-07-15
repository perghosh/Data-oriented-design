import docker
import time
import pyodbc
from pyodbc import Error

def wait_for_sqlserver(host, port, user, password, max_retries=30, delay=2):
    """Wait for SQL Server to be ready to accept connections"""
    for attempt in range(max_retries):
        try:
            conn_str = f"DRIVER={{ODBC Driver 17 for SQL Server}};SERVER={host},{port};UID={user};PWD={password};TrustServerCertificate=yes"
            conn = pyodbc.connect(conn_str)
            conn.close()
            return True
        except Error as e:
            print(f"Attempt {attempt + 1}/{max_retries}: SQL Server not ready yet... ({e})")
            time.sleep(delay)
        except Exception as e:
            print(f"Attempt {attempt + 1}/{max_retries}: SQL Server not ready yet... ({e})")
            time.sleep(delay)
    return False

def main():
    # Step 1: Pull the SQL Server image
    client = docker.from_env()
    print("Pulling mcr.microsoft.com/mssql/server:2022-latest image...")
    client.images.pull('mcr.microsoft.com/mssql/server:2022-latest')
    
    # Step 2: Run the container
    print("Starting sqlserver-test container...")
    try:
        # Remove existing container if it exists
        try:
            container = client.containers.get('sqlserver-test')
            container.stop()
            container.remove()
            print("Removed existing container")
        except docker.errors.NotFound:
            pass
        
        container = client.containers.run('mcr.microsoft.com/mssql/server:2022-latest', name='sqlserver-test', environment={'ACCEPT_EULA': 'Y', 'MSSQL_SA_PASSWORD': 'YourStrong@Passw0rd'}, ports={'1433/tcp': 1433}, detach=True)
        
        # Step 3: Wait for SQL Server to initialize
        print("Waiting for SQL Server to initialize...")
        if not wait_for_sqlserver('127.0.0.1', 1433, 'sa', 'YourStrong@Passw0rd'):
            print("SQL Server failed to start within the timeout period")
            return
        
        # Step 4: Connect to SQL Server and execute SQL commands
        print("Connecting to SQL Server and executing SQL commands...")
        sConnection = "DRIVER={ODBC Driver 17 for SQL Server};SERVER=127.0.0.1,1433;UID=sa;PWD=YourStrong@Passw0rd;TrustServerCertificate=yes"
        
        # Create database with autocommit enabled
        connection = pyodbc.connect(sConnection, autocommit=True)
        cursor = connection.cursor()
        cursor.execute("CREATE DATABASE test;")
        cursor.close()
        connection.close()
        
        # Connect to the test database for table operations
        conn_str_with_db = "DRIVER={ODBC Driver 17 for SQL Server};SERVER=127.0.0.1,1433;DATABASE=test;UID=sa;PWD=YourStrong@Passw0rd;TrustServerCertificate=yes"
        connection = pyodbc.connect(conn_str_with_db)
        cursor = connection.cursor()
        
        # Create table
        cursor.execute("""
            CREATE TABLE tdemo (
                demo_k INT IDENTITY(1,1) PRIMARY KEY,
                f_name NVARCHAR(50) NOT NULL
            );
        """)
        
        # Insert data
        cursor.execute("INSERT INTO tdemo (f_name) VALUES ('john_doe'), ('jane_smith');")
        connection.commit()  # Commit the transaction
        
        # Query data
        cursor.execute("SELECT * FROM tdemo;")
        print("Query results:")
        for row in cursor.fetchall():
            print(row)
        
        cursor.close()
        connection.close()
        
        print("\nDone. To stop and remove the container, run:")
        print("docker stop sqlserver-test && docker rm sqlserver-test")
        
    except Error as e:
        print(f"Database error: {e}")
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    main()