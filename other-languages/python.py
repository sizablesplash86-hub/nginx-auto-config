import os
import sys
import subprocess
import json
import urllib.request
import re
import socket
import fcntl
import struct

CURRENT_VERSION = "v2.0.0"
REPO_URL = "https://api.github.com/repos/sizablesplash86-hub/nginx-auto-config/releases/latest"

def root_check():
    if os.geteuid() != 0:
        print("\n\033[31mEnter root first\033[0m\n")
        return 1
    return 0

def update_check():
    try:
        req = urllib.request.Request(REPO_URL, headers={'User-Agent': 'auto-config-app'})
        with urllib.request.urlopen(req) as response:
            data = json.loads(response.read().decode())
            latest_version = data.get("tag_name", "")
            
            if latest_version and latest_version != CURRENT_VERSION:
                print(f"{CURRENT_VERSION} --> {latest_version}")
                ans = input("Would you like to upgrade? (y or n): ").strip()
                if ans == 'y':
                    print(f"\nUpgrading to {latest_version}...\n")
                    ver_num = latest_version[1:] if latest_version.startswith('v') else latest_version
                    update_cmd = (
                        f"curl -sfL https://github.com/sizablesplash86-hub/nginx-auto-config/releases/download/{latest_version}/auto-config_{ver_num}_amd64.deb -o /tmp/auto-config_update.deb && "
                        "dpkg -i /tmp/auto-config_update.deb || apt-get install -f -y && "
                        "rm -f /tmp/auto-config_update.deb"
                    )
                    if os.system(update_cmd) == 0:
                        print(f"\nUpgrade to {latest_version} completed successfully!\n")
                        os.system("auto-config")
                        sys.exit(0)
                    else:
                        print("\nUPGRADE FALIED! Continuing with current version...\n")
    except Exception:
        pass

def get_lan_ip():
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        ip = s.getsockname()[0]
        s.close()
        return ip
    except Exception:
        return "127.0.0.1"

def handle_error_and_rollback(avail_path, enabled_path):
    print("\033[31mUNKNOWN ERROR OCCURRED\033[0m")
    print("Removing broken configuration...")
    if os.path.exists(avail_path): os.unlink(avail_path)
    if os.path.islink(enabled_path) or os.path.exists(enabled_path): os.unlink(enabled_path)
    os.system("sudo nginx -t")

def handle_certbot_failure(domain_name, avail_path, enabled_path):
    print("\033[31mERROR\033[0m SSL certificate failed to deploy\n")
    rem = input("Would you like to remove config in /etc/nginx/sites-available /etc/nginx/sites-enabled? (y or n): ").strip()
    if rem == 'y':
        print("Removing broken configuration...")
        if os.path.exists(avail_path): os.unlink(avail_path)
        if os.path.islink(enabled_path) or os.path.exists(enabled_path): os.unlink(enabled_path)
        os.system("sudo nginx -t")
        sys.exit(0)
    elif rem in ['n', 'q']:
        print("Exiting now...\n")
        sys.exit(0)

def main():
    if root_check() != 0:
        sys.exit(0)

    update_check()
    lan_ip = get_lan_ip()

    print(f"\nWelcome to the NGINX Auto Config {CURRENT_VERSION}! Now known as N.A.P. for NGINX Auto Program because it's as easy as taking a NAP.\n")
    print(f"Visit \033[34mhttp://{lan_ip}:3487\033[0m in the browser to use the graphical interface\n")
    print("This program is designed to work with my server guide \033[34mhttps://www.sizablesplash.com/server-guide\033[0m\n")
    print("at any prompt, input q to exit\n")

    preset = input("Would you like to pick between presets? (y or n): ").strip()

    if preset == 'y':
        presets = input("Input 1 for Jellyfin, 2 for Nextcloud LEMP stacks, 3 for Plex, 4 for auto config GUI (NOT RECOMMENDED): ").strip()
        
        if presets == '1':
            domain_name = input("Enter domain name: ").strip()
            avail_path = "/etc/nginx/sites-available/jellyfin"
            enabled_path = "/etc/nginx/sites-enabled/jellyfin"

            config_content = f"""server {{
  server_name {domain_name};

  location / {{
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
  }}
}}
"""
            with open(avail_path, "w") as f:
                f.write(config_content)

            if not os.path.exists(enabled_path):
                os.symlink(avail_path, enabled_path)

            if os.system("sudo nginx -t") != 0:
                handle_error_and_rollback(avail_path, enabled_path)
                sys.exit(0)

            certbot_cmd = f"sudo certbot --nginx -d {domain_name}"
            if os.system(certbot_cmd) != 0:
                handle_certbot_failure(domain_name, avail_path, enabled_path)

            os.system("sudo nginx -t")
            os.system("sudo systemctl reload nginx")
            print(f"SSL certificate sucessfully deployed! Visit \033[34mhttps://{domain_name}\033[0m in your browser.\n\nExiting now...\n")
            sys.exit(0)

        elif presets == '2':
            print("Make sure to follow my LEMP Stacks guide at \033[34mhttps://www.sizablesplash.com/server-guide/prereqs/OS-config/nextcloud/LEMP-stacks\033[0m\n")
            try:
                php_ver = subprocess.check_output("php -r 'echo PHP_MAJOR_VERSION.\".\".PHP_MINOR_VERSION;' 2>/dev/null", shell=True).decode().strip()
            except Exception:
                php_ver = ""

            if not php_ver:
                print("\033[31mPHP not installed.\033[0m Exiting now...\n")
                sys.exit(0)

            php_sock = f"/run/php/php{php_ver}-fpm.sock"
            domain_name = input("Enter domain name: ").strip()

            avail_path = "/etc/nginx/sites-available/nextcloud"
            enabled_path = "/etc/nginx/sites-enabled/nextcloud"

            config_content = f"""upstream php-handler {{
  server unix:{php_sock};
}}

map $arg_v $asset_immutable {{
  "" "";
  default ", immutable";
}}

server {{
  listen 80;
  listen [::]:80;
  server_name {domain_name};
  server_tokens off;
  return 301 https://$server_name$request_uri;
}}

server {{
  http2 on;
  server_name {domain_name};
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
  types {{
    text/javascript mjs;
    application/wasm wasm;
  }}

  index index.php index.html /index.php$request_uri;

  location = / {{
    if ( $http_user_agent ~ ^DavClnt ) {{
      return 302 /remote.php/webdav/$is_args$args;
    }}
  }}

  location = /robots.txt {{
    allow all;
    log_not_found off;
    access_log off;
  }}

  location ^~ /.well-known {{
    location = /.well-known/carddav {{ return 301 /remote.php/dav/; }}
    location = /.well-known/caldav {{ return 301 /remote.php/dav/; }}
    location /.well-known/acme-challenge {{ try_files $uri $uri/ =404; }}
    location /.well-known/pki-validation {{ try_files $uri $uri/ =404; }}
    return 301 /index.php$request_uri;
  }}

  location ~ ^/(?:build|tests|config|lib|3rdparty|templates|data)(?:$|/) {{ return 404; }}
  location ~ ^/(?:\\.|autotest|occ|issue|indie|db_|console) {{ return 404; }}

  location ~ ^/(?:composer\\.(?:json|lock)|package(?:-lock)?\\.json|core/shipped\\.json)$ {{
    return 404;
  }}

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

  location ~ \\.(?:css|js|mjs|svg|gif|ico|jpg|png|webp|wasm|tflite|map|ogg|flac|mp4|webm)$ {{
    try_files $uri /index.php$request_uri;
    add_header Cache-Control "public, max-age=15778463$asset_immutable" always;
    add_header Referrer-Policy "no-referrer" always;
    add_header X-Content-Type-Options "nosniff" always;
    add_header X-Frame-Options "SAMEORIGIN" always;
    add_header X-Permitted-Cross-Domain-Policies "none" always;
    add_header X-Robots-Tag "noindex, nofollow" always;
    access_log off;
  }}

  location ~ \\.(otf|woff2?)$ {{
    try_files $uri /index.php$request_uri;
    expires 7d;
    access_log off;
  }}

  location /remote {{
    return 301 /remote.php$request_uri;
  }}

  location / {{
    try_files $uri $uri/ /index.php$request_uri;
  }}
}}
"""
            with open(avail_path, "w") as f:
                f.write(config_content)

            if not os.path.exists(enabled_path):
                os.symlink(avail_path, enabled_path)

            if os.system("sudo nginx -t") != 0:
                handle_error_and_rollback(avail_path, enabled_path)
                sys.exit(0)

            certbot_cmd = f"sudo certbot --nginx -d {domain_name}"
            if os.system(certbot_cmd) != 0:
                handle_certbot_failure(domain_name, avail_path, enabled_path)
            else:
                print(f"SSL certificate sucessfully deployed! Visit \033[34mhttps://{domain_name}\033[0m in your browser.\n\nExiting now...\n")
                os.system("sudo nginx -t")
                os.system("sudo systemctl reload nginx")

            sys.exit(0)

        elif presets == '3':
            domain_name = input("Enter domain name: ").strip()
            avail_path = "/etc/nginx/sites-available/plex"
            enabled_path = "/etc/nginx/sites-enabled/plex"

            config_content = f"""server {{
  listen 80;
  server_name {domain_name};

  location / {{
    proxy_pass http://127.0.0.1:3400;
    proxy_http_version 1.1;
    proxy_set_header Upgrade $http_upgrade;
    proxy_set_header Connection 'upgrade';
    proxy_set_header Host $host;
    proxy_cache_bypass $http_upgrade;
    proxy_set_header X-Real-IP $remote_addr;
  }}

  error_page 502 /502.html;
  location = /502.html {{
    root /home/PlexStore;
  }}
}}
"""
            with open(avail_path, "w") as f:
                f.write(config_content)

            if not os.path.exists(enabled_path):
                os.symlink(avail_path, enabled_path)

            if os.system("sudo nginx -t") != 0:
                handle_error_and_rollback(avail_path, enabled_path)
                sys.exit(0)

            certbot_cmd = f"sudo certbot --nginx -d {domain_name}"
            if os.system(certbot_cmd) != 0:
                handle_certbot_failure(domain_name, avail_path, enabled_path)

            os.system("sudo nginx -t")
            os.system("sudo systemctl reload nginx")
            print(f"SSL certificate successfully deployed! Visit \033[0mhttps://{domain_name}\033[0m in you browser.\n")
            sys.exit(0)

        elif presets == '4':
            gui = input("\033[31mWARNING!!\033[0m Opening the auto config GUI to the public internet is risky. Do you wish to proceed? (y or n): ").strip()
            if gui == 'n':
                sys.exit(0)

            domain_name = input("Enter domain name: ").strip()
            os.system("rm -f /etc/nginx/sites-available/auto-config /etc/nginx/sites-enabled/auto-config")

            avail_path = "/etc/nginx/sites-available/auto-config"
            enabled_path = "/etc/nginx/sites-enabled/auto-config"

            config_content = f"""server {{
  listen 80;
  listen [::]:80;

  server_name {domain_name};

  root /var/www/auto-config/;
  index index.html index.php;

  location / {{
    autoindex on;
    try_files $uri $uri/ $uri/index.html $uri.html $uri.php =404;
  }}
}}
"""
            with open(avail_path, "w") as f:
                f.write(config_content)

            if not os.path.exists(enabled_path):
                os.symlink(avail_path, enabled_path)

            if os.system("sudo nginx -t") != 0:
                handle_error_and_rollback(avail_path, enabled_path)
                sys.exit(0)

            certbot_cmd = f"sudo certbot --nginx -d {domain_name}"
            if os.system(certbot_cmd) != 0:
                handle_certbot_failure(domain_name, avail_path, enabled_path)
            else:
                print(f"SSL certificate sucessfully deployed! Visit \033[34mhttps://{domain_name}\033[0m in your browser.\n\nExiting now...\n")
                os.system("sudo nginx -t")
                os.system("sudo systemctl reload nginx")

            sys.exit(0)

    if preset == 'q':
        sys.exit(0)

    proxy = ""
    directory = ""
    config_type = input("Input 1 for reverse proxy or 2 for standalone directory: ").strip()

    if config_type == '1':
        proxy = input("Enter the proxy port: ").strip()
    elif config_type == '2':
        directory = input("Enter directory path: ").strip()

    config_name = input("Enter name of the config: ").strip()
    domain_name = input("Enter domain name: ").strip()

    avail_path = f"/etc/nginx/sites-available/{config_name}"
    enabled_path = f"/etc/nginx/sites-enabled/{config_name}"

    if config_type == '1':
        config_content = f"""server {{
  server_name {domain_name};

  location / {{
    proxy_pass http://127.0.0.1:{proxy};
    proxy_set_header Host $host;
    proxy_set_header X-Real-IP $remote_addr;
    proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
    proxy_set_header X-Forwarded-Proto $scheme;
    proxy_set_header X-Forwarded-Protocol $scheme;
    proxy_set_header X-Forwarded-Host $http_host;
    proxy_http_version 1.1;
    proxy_set_header Upgrade $http_upgrade;
    proxy_set_header Connection "upgrade";
  }}
}}
"""
    elif config_type == '2':
        has_php = os.system("php -v 1>/dev/null 2>&1") == 0
        if has_php:
            config_content = f"""server {{
  listen 80;
  listen [::]:80;

  server_name {domain_name};

  root {directory};
  index index.html index.htm index.php;

  location / {{
    autoindex on;
    try_files $uri $uri/ $uri/index.html $uri.html $uri.php =404;
  }}
}}
"""
        else:
            config_content = f"""server {{
  listen 80;
  listen [::]:80;

  server_name {domain_name};

  root {directory};
  index index.html index.htm;

  location / {{
    autoindex on;
    try_files $uri$uri/ $uri/index.html $uri.html =404;
  }}
}}
"""

    with open(avail_path, "w") as f:
        f.write(config_content)

    if not os.path.exists(enabled_path):
        os.symlink(avail_path, enabled_path)

    if os.system("sudo nginx -t") != 0:
        handle_error_and_rollback(avail_path, enabled_path)
        sys.exit(0)

    certbot_cmd = f"sudo certbot --nginx -d {domain_name}"
    if os.system(certbot_cmd) != 0:
        handle_certbot_failure(domain_name, avail_path, enabled_path)
    else:
        print(f"SSL certificate sucessfully deployed! Visit \033[34mhttps://{domain_name}\033[0m in your browser.\n\nExiting now...\n")
        os.system("sudo nginx -t")
        os.system("sudo systemctl reload nginx")

if __name__ == "__main__":
    main()
