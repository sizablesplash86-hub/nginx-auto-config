const express = require('express');
const { exec, execSync } = require('child_process');
const fs = require('fs');
const path = require('path');
const os = require('os');

const app = express();
const PORT = 3487;
const CURRENT_VERSION = "v2.0.0";

app.use(express.json());
app.use(express.static(__dirname));

// Verify root execution
if (process.getuid && process.getuid() !== 0) {
    console.error("\x1b[31mError: Root privileges required to write NGINX configs.\x1b[0m");
    process.exit(1);
}

// LAN IP Lookup
function getLanIp() {
    const interfaces = os.networkInterfaces();
    for (const name of Object.keys(interfaces)) {
        if (name.startsWith('lo') || name.startsWith('wg') || name.startsWith('tailscale') || name.startsWith('docker') || name.startsWith('veth')) {
            continue;
        }
        for (const net of interfaces[name]) {
            if (net.family === 'IPv4' && !net.internal) {
                return net.address;
            }
        }
    }
    return 'localhost';
}

// Deploy NGINX config, test, symlink, and run certbot
function deployNginxConfig(configName, domain, content, res) {
    const availPath = `/etc/nginx/sites-available/${configName}`;
    const enabledPath = `/etc/nginx/sites-enabled/${configName}`;

    try {
        fs.writeFileSync(availPath, content);
        if (!fs.existsSync(enabledPath)) {
            fs.symlinkSync(availPath, enabledPath);
        }

        execSync('sudo nginx -t');

        const certbotCmd = `sudo certbot --nginx -d ${domain}`;
        exec(certbotCmd, (certError) => {
            if (certError) {
                if (fs.existsSync(availPath)) fs.unlinkSync(availPath);
                if (fs.existsSync(enabledPath)) fs.unlinkSync(enabledPath);
                execSync('sudo nginx -t');
                return res.json({ success: false, message: 'SSL Certificate deployment failed. Rolling back configuration.' });
            }

            execSync('sudo nginx -t');
            execSync('sudo systemctl reload nginx');
            return res.json({ success: true, message: `SSL certificate deployed! Visit https://${domain}` });
        });

    } catch (err) {
        if (fs.existsSync(availPath)) fs.unlinkSync(availPath);
        if (fs.existsSync(enabledPath)) fs.unlinkSync(enabledPath);
        exec('sudo nginx -t');
        return res.json({ success: false, message: `NGINX configuration test failed: ${err.message}` });
    }
}

app.post('/api/deploy', (req, res) => {
    const { mode, preset, type, configName, domain, proxy, directory } = req.body;

    if (mode === 'preset') {
        if (preset === '1') { // Jellyfin
            const content = `server {
    server_name ${domain};

    location / {
        proxy_pass http://127.0.0.1:8096;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
        proxy_set_header X-Forwarded-Protocol $scheme;
        proxy_set_header X-Forwarded-Host $http_host;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection "upgrade";
    }
}`;
            return deployNginxConfig('jellyfin', domain, content, res);
        } 
        
        else if (preset === '2') { // Nextcloud
            let phpVer = '';
            try {
                phpVer = execSync("php -r 'echo PHP_MAJOR_VERSION.\".\".PHP_MINOR_VERSION;' 2>/dev/null").toString().trim();
            } catch (e) {
                return res.json({ success: false, message: 'PHP is not installed on this server.' });
            }

            const phpSock = `/run/php/php${phpVer}-fpm.sock`;
            const content = `upstream php-handler {
    server unix:${phpSock};
}

map $arg_v $asset_immutable {
    "" "";
    default ", immutable";
}

server {
    listen 80;
    listen [::]:80;
    server_name ${domain};
    server_tokens off;
    return 301 https://$server_name$request_uri;
}

server {
    http2 on;
    server_name ${domain};
    root /var/www/nextcloud;
    server_tokens off;

    client_max_body_size 512M;
    client_body_timeout 300s;
    fastcgi_buffers 64 4K;

    gzip on;
    gzip_vary on;
    gzip_comp_level 4;
    gzip_min_length 256;
    gzip_proxied expired no-cache no-store private no_last_modified no_etag auth;
    gzip_types application/atom+xml text/javascript application/javascript application/json application/ld+json application/manifest+json application/rss+xml application/vnd.geo+json application/vnd.ms-fontobject application/wasm application/x-font-ttf application/x-web-app-manifest+json application/xhtml+xml application/xml font/opentype image/bmp image/svg+xml image/x-icon text/cache-manifest text/css text/plain text/vcard text/vnd.rim.location.xloc text/vtt text/x-component text/x-cross-domain-policy;

    client_body_buffer_size 512k;

    add_header Referrer-Policy "no-referrer" always;
    add_header X-Content-Type-Options "nosniff" always;
    add_header X-Frame-Options "SAMEORIGIN" always;
    add_header X-Permitted-Cross-Domain-Policies "none" always;
    add_header X-Robots-Tag "noindex, nofollow" always;
    add_header Permissions-Policy "camera=(), microphone=(), geolocation=()" always;
    add_header Content-Security-Policy "default-src 'self'; script-src 'self' 'unsafe-inline' 'unsafe-eval'; style-src 'self' 'unsafe-inline'; img-src 'self' data: blob:; font-src 'self'; connect-src 'self'; frame-ancestors 'self'" always;

    fastcgi_hide_header X-Powered-By;

    include mime.types;
    types {
        text/javascript mjs;
        application/wasm wasm;
    }

    index index.php index.html /index.php$request_uri;

    location = / {
        if ( $http_user_agent ~ ^DavClnt ) {
            return 302 /remote.php/webdav/$is_args$args;
        }
    }

    location = /robots.txt {
        allow all;
        log_not_found off;
        access_log off;
    }

    location ^~ /.well-known {
        location = /.well-known/carddav { return 301 /remote.php/dav/; }
        location = /.well-known/caldav { return 301 /remote.php/dav/; }
        location /.well-known/acme-challenge { try_files $uri $uri/ =404; }
        location /.well-known/pki-validation { try_files $uri $uri/ =404; }
        return 301 /index.php$request_uri;
    }

    location ~ ^/(?:build|tests|config|lib|3rdparty|templates|data)(?:$|/) { return 404; }
    location ~ ^/(?:\\.|autotest|occ|issue|indie|db_|console) { return 404; }

    location ~ ^/(?:composer\\.(?:json|lock)|package(?:-lock)?\\.json|core/shipped\\.json)$ {
        return 404;
    }

    rewrite ^/(?!index|remote|public|cron|status|ocs\\/v[12]|ocs-provider\\/.+|core\\/ajax\\/update|updater\\/.+|.+\\/richdocumentscode(_arm64)?\\/proxy) /index.php$request_uri;

    fastcgi_split_path_info ^(.+?\\.php)(/.*)$;
    set $path_info $fastcgi_path_info;

    try_files $fastcgi_script_name =404;

    include fastcgi_params;
    fastcgi_pass php-handler;

    fastcgi_param SCRIPT_FILENAME $document_root$fastcgi_script_name;
    fastcgi_param PATH_INFO $path_info;
    fastcgi_param HTTPS on;
    fastcgi_param modHeadersAvailable true;
    fastcgi_param front_controller_active true;
    fastcgi_max_temp_file_size 0;

    location ~ \\.(?:css|js|mjs|svg|gif|ico|jpg|png|webp|wasm|tflite|map|ogg|flac|mp4|webm)$ {
        try_files $uri /index.php$request_uri;
        add_header Cache-Control "public, max-age=15778463$asset_immutable" always;
        add_header Referrer-Policy "no-referrer" always;
        add_header X-Content-Type-Options "nosniff" always;
        add_header X-Frame-Options "SAMEORIGIN" always;
        add_header X-Permitted-Cross-Domain-Policies "none" always;
        add_header X-Robots-Tag "noindex, nofollow" always;
        access_log off;
    }

    location ~ \\.(otf|woff2?)$ {
        try_files $uri /index.php$request_uri;
        expires 7d;
        access_log off;
    }

    location /remote {
        return 301 /remote.php$request_uri;
    }

    location / {
        try_files $uri $uri/ /index.php$request_uri;
    }
}`;
            return deployNginxConfig('nextcloud', domain, content, res);
        }

        else if (preset === '3') { // Plex
            const content = `server {
    listen 80;
    server_name ${domain};

    location / {
        proxy_pass http://127.0.0.1:3400;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection 'upgrade';
        proxy_set_header Host $host;
        proxy_cache_bypass $http_upgrade;
        proxy_set_header X-Real-IP $remote_addr;
    }

    error_page 502 /502.html;
    location = /502.html {
        root /home/PlexStore;
    }
}`;
            return deployNginxConfig('plex', domain, content, res);
        }

        else if (preset === '4') { // GUI
            const content = `server {
    listen 80;
    listen [::]:80;

    server_name ${domain};

    root /var/www/auto-config/;
    index index.html index.php;

    location / {
        autoindex on;
        try_files $uri $uri/ $uri/index.html $uri.html $uri.php =404;
    }
}`;
            return deployNginxConfig('auto-config', domain, content, res);
        }
    } 
    
    else if (mode === 'custom') {
        let content = '';
        if (type === '1') { // Reverse Proxy
            content = `server {
    server_name ${domain};

    location / {
        proxy_pass http://127.0.0.1:${proxy};
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
        proxy_set_header X-Forwarded-Protocol $scheme;
        proxy_set_header X-Forwarded-Host $http_host;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection "upgrade";
    }
}`;
        } else { // Directory
            let hasPhp = false;
            try {
                execSync('php -v 1>/dev/null 2>&1');
                hasPhp = true;
            } catch (e) {}

            content = `server {
    listen 80;
    listen [::]:80;

    server_name ${domain};

    root ${directory};
    index index.html index.htm${hasPhp ? ' index.php' : ''};

    location / {
        autoindex on;
        try_files $uri $uri/ $uri/index.html ${hasPhp ? '$uri.php ' : ''}=404;
    }
}`;
        }
        return deployNginxConfig(configName, domain, content, res);
    }
});

app.listen(PORT, () => {
    console.log(`\nNGINX Auto Program ${CURRENT_VERSION} (N.A.P.)`);
    console.log(`Web interface ready: http://${getLanIp()}:${PORT}\n`);
});
