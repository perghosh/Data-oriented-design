# HTTP Server Setup

## Multi-instance setup (Debian + Caddy reverse proxy)

This setup runs multiple HTTP server instances from one binary, with one shared web root and one database file per instance.

### Example directory layout

~~~text
/opt/http/
├── http                           # C++ server binary
├── vote/                          # Shared web root (HTML/CSS/JS)
│   ├── js/
│   ├── css/
│   ├── index.html
│   ├── vote-statements.xml        # Shared SQL statements
│   └── vote-expressions.xml       # Shared expressions
└── config/
    ├── configuration-test.xml     # Test system
    ├── configuration-kt.xml       # Knapptryckarna
    ├── configuration-afs.xml      # Alternativ för Sverige
    └── configuration-as.xml       # Ambition Sverige

/var/lib/http/                     # Database directory
├── test.sqlite
├── kt.sqlite
├── afs.sqlite
└── as.sqlite
~~~

### Required permissions

~~~bash
sudo mkdir -p /var/lib/http
sudo chown -R www-data:www-data /var/lib/http
~~~

### Start one instance manually

~~~bash
/opt/http/http --configuration "/opt/http/config/configuration-test.xml"
~~~

---

## systemd template service

Create `/etc/systemd/system/http@.service`:

~~~ini
[Unit]
Description=HTTP Server instance for %I
After=network.target

[Service]
Type=simple
WorkingDirectory=/opt/http
# %I is replaced by the instance name (for example: test, kt, afs, as)
ExecStart=/opt/http/http --configuration /opt/http/config/configuration-%I.xml
Restart=always
User=www-data
Group=www-data

[Install]
WantedBy=multi-user.target
~~~

### Enable and start instances

~~~bash
sudo systemctl daemon-reload
sudo systemctl enable --now http@test
sudo systemctl enable --now http@kt
sudo systemctl enable --now http@afs
sudo systemctl enable --now http@as
~~~

### Verify status and logs

~~~bash
systemctl status http@test
journalctl -u http@test -f
~~~


## Caddy reverse proxy configuration

Edit `/etc/caddy/Caddyfile` to route incoming traffic to the correct backend port based on your setup. 

Ensure that the port numbers below match the `<property key="port" value="..." />` defined inside each instance's XML configuration file.

### Alternative A: Route via URL Paths (Single Domain)
If you want all systems accessible under one single root domain (e.g., `https://rootsystem.com`):

```caddy
rootsystem.com {
    # Route for Test (Port 8001)
    handle_path /test* {
        reverse_proxy localhost:8001
    }

    # Route for Knapptryckarna (Port 8002)
    handle_path /kt* {
        reverse_proxy localhost:8002
    }

    # Route for Alternativ för Sverige (Port 8003)
    handle_path /afs* {
        reverse_proxy localhost:8003
    }

    # Route for Ambition Sverige (Port 8004)
    handle_path /as* {
        reverse_proxy localhost:8004
    }
}
```
*Note: `handle_path` automatically strips the prefix (like `/kt`) before passing the request to your C++ binary. Your binary will safely see the request as `/index.html`.*

### Alternative B: Route via Unique Domains (Subdomains/Custom Domains)
If each instance should have its own dedicated domain or subdomain:

```caddy
://rootsystem.com {
    reverse_proxy localhost:8001
}

knapptryckarna.se, www.knapptryckarna.se {
    reverse_proxy localhost:8002
}

alternativforsverige.se {
    reverse_proxy localhost:8003
}

ambitionsverige.se {
    reverse_proxy localhost:8004
}
```

### Apply Caddy changes
After modifying the configuration, reload Caddy to apply the routing without downtime:

```bash
sudo systemctl reload caddy
```
