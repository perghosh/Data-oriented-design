# 1. Switch to Debian-based nginx for better Ubuntu binary compatibility
FROM nginx:latest

# 2. Install necessary libraries for C++ and bash ----------------------------
RUN apt-get update && apt-get install -y \
    libc6 \
    libstdc++6 \
    bash \
    sed \
    neovim \
    && rm -rf /var/lib/apt/lists/*

# Copy the cleaner binary -----------------------------------------------------
COPY cleaner /usr/local/bin/cleaner
RUN chmod +x /usr/local/bin/cleaner

# Copy the bat binary used for syntax highlighting in the output --------------
COPY bat /usr/local/bin/bat
RUN chmod +x /usr/local/bin/bat


# Install script to download GitHub repos
COPY github-load.sh /usr/local/bin/github-load.sh
RUN chmod +x /usr/local/bin/github-load.sh

# Create directory structure
RUN mkdir -p /app/www /app/output /data/work

# Copy nginx configuration
COPY nginx.conf /etc/nginx/conf.d/default.conf

# --- HTML Files Generation ---
RUN printf "<!DOCTYPE html><html><head><title>Cleaner</title></head><body><h1>🔍 Cleaner - Code Analysis Tool</h1></body></html>" > /app/www/index.html
RUN printf "<!DOCTYPE html><html><body><h1>404 - Not Found</h1></body></html>" > /app/www/404.html
RUN printf "<!DOCTYPE html><html><body><h1>50x - Server Error</h1></body></html>" > /app/www/50x.html

# 3. Create startup script using printf to force Linux-style line endings (\n)
RUN printf '#!/bin/bash\n\
set -e\n\
echo "Starting Cleaner Application with Nginx..."\n\
echo "Cleaner binary location: /usr/local/bin/cleaner"\n\
# Start nginx in background\n\
nginx -g "daemon off;" &\n\
# Keep container running\n\
while true; do\n\
    sleep 3600\n\
done\n' > /app/start.sh

# 4. CRITICAL: Scrub any potential Windows characters and fix permissions
RUN sed -i 's/\r$//' /app/start.sh
RUN chmod +x /app/start.sh

EXPOSE 80

# 5. Healthcheck using bash socket (reliable on Debian)
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD bash -c 'printf "GET / HTTP/1.1\nHost: localhost\n\n" > /dev/tcp/127.0.0.1/80' || exit 1

CMD ["/app/start.sh"]
