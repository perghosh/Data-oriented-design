FROM nginx:alpine

# Install necessary packages
RUN apk add --no-cache libc6-compat bash

# Copy the cleaner binary to system path for global access
COPY cleaner /usr/local/bin/cleaner

# Make cleaner executable
RUN chmod +x /usr/local/bin/cleaner

# Create directory structure for web content and output
RUN mkdir -p /app/www /app/output /data/work

# Copy nginx configuration
COPY nginx.conf /etc/nginx/conf.d/default.conf

# Create index.html page
RUN cat > /app/www/index.html << 'EOF'
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Cleaner - Code Analysis Tool</title>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            margin: 0;
            padding: 20px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
            background: white;
            padding: 30px;
            border-radius: 10px;
            box-shadow: 0 10px 40px rgba(0,0,0,0.2);
        }
        h1 {
            color: #667eea;
            border-bottom: 3px solid #667eea;
            padding-bottom: 10px;
        }
        .section {
            margin: 30px 0;
            padding: 20px;
            background: #f8f9fa;
            border-radius: 8px;
            border-left: 4px solid #667eea;
        }
        .command {
            background: #2d3748;
            color: #48bb78;
            padding: 15px;
            border-radius: 5px;
            font-family: 'Courier New', monospace;
            overflow-x: auto;
            margin: 10px 0;
        }
        .info-box {
            background: #e3f2fd;
            border: 1px solid #2196f3;
            padding: 15px;
            border-radius: 5px;
            margin: 15px 0;
        }
        button {
            background: #667eea;
            color: white;
            border: none;
            padding: 12px 24px;
            border-radius: 5px;
            cursor: pointer;
            font-size: 14px;
            margin: 5px;
        }
        button:hover {
            background: #5568d3;
        }
        #results {
            background: #1a202c;
            color: #e2e8f0;
            padding: 20px;
            border-radius: 5px;
            max-height: 500px;
            overflow-y: auto;
            font-family: 'Courier New', monospace;
            white-space: pre-wrap;
            margin-top: 20px;
        }
        .grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
            margin-top: 20px;
        }
        .card {
            background: white;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }
        .card h3 {
            color: #667eea;
            margin-top: 0;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🔍 Cleaner - Code Analysis Tool</h1>

        <div class="info-box">
            <strong>💡 Note:</strong> Cleaner runs on <strong>any directory</strong> inside the pod. Use kubectl exec to run analysis on mounted volumes or any path.
        </div>

        <div class="grid">
            <div class="card">
                <h3>📋 Quick Commands</h3>
                <p>Run these commands in your terminal:</p>

                <div class="section">
                    <strong>Execute Cleaner on Any Directory:</strong>
                    <div class="command">
kubectl exec -it deployment/cleaner-112 -- cleaner count --source /path/to/code --recursive 3
                    </div>
                </div>

                <div class="section">
                    <strong>Execute Cleaner on Mounted Volume:</strong>
                    <div class="command">
kubectl exec -it deployment/cleaner-112 -- cleaner count --source /data/work --recursive 2
                    </div>
                </div>

                <div class="section">
                    <strong>Save Output to File:</strong>
                    <div class="command">
kubectl exec deployment/cleaner-112 -- cleaner count --source /app/www --recursive 1 > /app/output/results.txt
                    </div>
                </div>

                <div class="section">
                    <strong>Copy Output to Local Machine:</strong>
                    <div class="command">
kubectl exec deployment/cleaner-112 -- cat /app/output/results.txt > local_results.txt
                    </div>
                </div>
            </div>

            <div class="card">
                <h3>🎯 Available Commands</h3>
                <ul style="line-height: 2;">
                    <li><code>count</code> - Count lines in files</li>
                    <li><code>list</code> - List lines matching patterns</li>
                    <li><code>find</code> - Search for patterns in files</li>
                    <li><code>dir</code> - List files in directories</li>
                    <li><code>history</code> - View command history</li>
                    <li><code>config</code> - Manage configuration</li>
                </ul>

                <div class="section" style="margin-top: 20px;">
                    <strong>Help:</strong>
                    <div class="command">
kubectl exec -it deployment/cleaner-112 -- cleaner help
                    </div>
                    <div class="command">
kubectl exec -it deployment/cleaner-112 -- cleaner count --help
                    </div>
                </div>
            </div>
        </div>

        <div class="section">
            <h2>📊 Latest Results</h2>
            <button onclick="loadResults()">Load Results</button>
            <button onclick="clearResults()">Clear</button>
            <div id="results">No results loaded. Click "Load Results" to fetch the latest output.</div>
        </div>

        <div class="section">
            <h2>📁 Output Files</h2>
            <p>Generated files are stored in <code>/app/output</code> and accessible via:</p>
            <div class="command">
http://service-ip/output/
            </div>
        </div>

        <div class="section">
            <h2>🔌 Access the Web Interface</h2>
            <p>Get the service URL:</p>
            <div class="command">
minikube service cleaner-112-service
            </div>
            <p>or</p>
            <div class="command">
kubectl get svc cleaner-112-service
            </div>
        </div>
    </div>

    <script>
        async function loadResults() {
            const resultsDiv = document.getElementById('results');
            resultsDiv.innerHTML = 'Loading...';

            try {
                const response = await fetch('/api/results');
                const text = await response.text();
                resultsDiv.innerHTML = text || 'No results file found.';
            } catch (error) {
                resultsDiv.innerHTML = 'Error loading results: ' + error.message;
            }
        }

        function clearResults() {
            document.getElementById('results').innerHTML = 'No results loaded. Click "Load Results" to fetch the latest output.';
        }
    </script>
</body>
</html>
EOF

# Create 404.html
RUN cat > /app/www/404.html << 'EOF'
<!DOCTYPE html>
<html>
<head>
    <title>404 - Page Not Found</title>
    <style>
        body { font-family: Arial, sans-serif; text-align: center; padding: 50px; }
        h1 { color: #667eea; }
        a { color: #667eea; }
    </style>
</head>
<body>
    <h1>404 - Page Not Found</h1>
    <p>The page you are looking for does not exist.</p>
    <p><a href="/">Go to Home</a></p>
</body>
</html>
EOF

# Create 50x.html
RUN cat > /app/www/50x.html << 'EOF'
<!DOCTYPE html>
<html>
<head>
    <title>Server Error</title>
    <style>
        body { font-family: Arial, sans-serif; text-align: center; padding: 50px; }
        h1 { color: #e53e3e; }
        a { color: #667eea; }
    </style>
</head>
<body>
    <h1>Server Error</h1>
    <p>Something went wrong on our end.</p>
    <p><a href="/">Go to Home</a></p>
</body>
</html>
EOF

# Create startup script that keeps nginx running and allows cleaner execution
RUN cat > /app/start.sh << 'EOF'
#!/bin/sh
set -e

echo "Starting Cleaner Application with Nginx..."
echo "Cleaner binary location: /usr/local/bin/cleaner"
echo "Web interface: http://localhost:80"
echo ""
echo "To run cleaner on any directory, use:"
echo "kubectl exec -it <pod-name> -- cleaner <command> --source /path/to/directory"
echo ""

# Start nginx in foreground
echo "Starting nginx..."
nginx -g "daemon off;" &

# Keep container running for kubectl exec access
echo "Container is ready. Cleaner can be executed on any directory via kubectl exec."
while true; do
    sleep 3600
done
EOF

RUN chmod +x /app/start.sh

# Expose nginx port
EXPOSE 80

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD wget --no-verbose --tries=1 --spider http://localhost/health || exit 1

# Start the container
CMD ["/app/start.sh"]
