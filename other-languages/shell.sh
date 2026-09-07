#!/usr/bin/env bash

CURRENT_VERSION="v2.0.0"
REPO_URL="https://api.github.com/repos/sizablesplash86-hub/nginx-auto-config/releases/latest"

root_check() {
    if [ "$(id -u)" -ne 0 ]; then
        echo -e "\n\033[31mEnter root first\033[0m\n"
        return 1
    fi
    return 0
}

update_check() {
    local latest_version
    latest_version=$(curl -s -H "User-Agent: auto-config-app" "$REPO_URL" | grep '"tag_name":' | sed -E 's/.*"([^"]+)".*/\1/')
    
    if [ -n "$latest_version" ] && [ "$latest_version" != "$CURRENT_VERSION" ]; then
        echo -n "$CURRENT_VERSION --> $latest_version"
        read -p "Would you like to upgrade? (y or n): " ans
        if [ "$ans" = "y" ]; then
            echo -e "\nUpgrading to $latest_version...\n"
            local ver_num="${latest_version#v}"
            local update_cmd="curl -sfL https://github.com/sizablesplash86-hub/nginx-auto-config/releases/download/${latest_version}/auto-config_${ver_num}_amd64.deb -o /tmp/auto-config_update.deb && dpkg -i /tmp/auto-config_update.deb || apt-get install -f -y && rm -f /tmp/auto-config_update.deb"
            
            if eval "$update_cmd"; then
                echo -e "\nUpgrade to $latest_version completed successfully!\n"
                auto-config
                exit 0
            else
                echo -e "\nUPGRADE FALIED! Continuing with current version...\n"
            fi
        fi
    fi
}

get_lan_ip() {
    local ip
    ip=$(hostname -I | awk '{print $1}')
    echo "${ip:-127.0.0.1}"
}

handle_error() {
    local avail_path="$1"
    local enabled_path="$2"
    echo -e "\033[31mUNKNOWN ERROR OCCURRED\033[0m"
    echo "Removing broken configuration..."
    rm -f "$avail_path" "$enabled_path"
    sudo nginx -t
    exit 0
}

handle_certbot_error() {
    local avail_path="$1"
    local enabled_path="$2"
    echo -e "\033[31mERROR\033[0m SSL certificate failed to deploy\n"
    read -p "Would you like to remove config in /etc/nginx/sites-available /etc/nginx/sites-enabled? (y or n): " rem
    if [ "$rem" = "y" ]; then
        echo "Removing broken configuration..."
        rm -f "$avail_path" "$enabled_path"
        sudo nginx -t
        exit 0
    fi
    if [ "$rem" = "n" ] || [ "$rem" = "q" ]; then
        echo -e "Exiting now...\n"
        exit 0
    fi
}

main() {
    root_check || exit 0
    update_check

    lan_ip=$(get_lan_ip)

    echo -e "\nWelcome to the NGINX Auto Config $CURRENT_VERSION! Now known as N.A.P. for NGINX Auto Program because it's as easy as taking a NAP.\n"
    echo -e "Visit \033[34mhttp://${lan_ip}:3487\033[0m in the browser to use the graphical interface\n"
    echo -e "This program is designed to work with my server guide \033[34mhttps://www.sizablesplash.com/server-guide\033[0m\n"
    echo -e "at any prompt, input q to exit\n"

    read -p "Would you like to pick between presets? (y or n): " preset

    if [ "$preset" = "y" ]; then
        read -p "Input 1 for Jellyfin, 2 for Nextcloud LEMP stacks, 3 for Plex, 4 for auto config GUI (NOT RECOMMENDED): " presets

        if [ "$presets" = "1" ]; then
            read -p "Enter domain name: " domain_name
            avail_path="/etc/nginx/sites-available/jellyfin"
            enabled_path="/etc/nginx/sites-enabled/jellyfin"

            cat <<EOF > "$avail_path"
server {
  server_name $domain_name;

  location / {
    proxy_pass http://127.0.0.1:8096;
    proxy_set_header Host \$host;
    proxy_set_header X-Real-IP \$remote_addr;
    proxy_set_header X-Forwarded-For \$proxy_add_x_forwarded_for;
    proxy_set_header X-Forwarded-Proto \$scheme;
    proxy_set_header X-Forwarded-Protocol \$scheme;
    proxy_set_header X-Forwarded-Host \$http_host;
    proxy_http_version 1.1;
    proxy_set_header Upgrade \$http_upgrade;
    proxy_set_header Connection "upgrade";
  }
}
EOF
            ln -sf "$avail_path" "$enabled_path"

            if ! sudo nginx -t; then
                handle_error "$avail_path" "$enabled_path"
            fi

            if ! sudo certbot --nginx -d "$domain_name"; then
                handle_certbot_error "$avail_path" "$enabled_path"
            fi

            sudo nginx -t
            sudo systemctl reload nginx
            echo -e "SSL certificate sucessfully deployed! Visit \033[34mhttps://${domain_name}\033[0m in your browser.\n\nExiting now...\n"
            exit 0
        fi

        if [ "$presets" = "2" ]; then
            echo -e "Make sure to follow my LEMP Stacks guide at \033[34mhttps://www.sizablesplash.com/server-guide/prereqs/OS-config/nextcloud/LEMP-stacks\033[0m\n"
            php_ver=$(php -r 'echo PHP_MAJOR_VERSION.".".PHP_MINOR_VERSION;' 2>/dev/null)
            if [ -z "$php_ver" ]; then
                echo -e "\033[31mPHP not installed.\033[0m Exiting now...\n"
                exit 0
            fi

            php_sock="/run/php/php${php_ver}-fpm.sock"
            read -p "Enter domain name: " domain_name
            avail_path="/etc/nginx/sites-available/nextcloud"
            enabled_path="/etc/nginx/sites-enabled/nextcloud"

            cat <<EOF > "$avail_path"
upstream php-handler {
  server unix:${php_sock};
}

map \$arg_v \$asset_immutable {
  "" "";
  default ", immutable";
}

server {
  listen 80;
  listen [::]:80;
  server_name ${domain_name};
  server_tokens off;
  return 301 https://\$server_name\$request_uri;
}

server {
  http2 on;
  server_name ${domain_name};
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

  index index.php index.html /index.php\$request_uri;

  location = / {
    if ( \$http_user_agent ~ ^DavClnt ) {
      return 302 /remote.php/webdav/\$is_args\$args;
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
    location /.well-known/acme-challenge { try_files \$uri \$uri/ =404; }
    location /.well-known/pki-validation { try_files \$uri \$uri/ =404; }
    return 301 /index.php\$request_uri;
  }

  location ~ ^/(?:build|tests|config|lib|3rdparty|templates|data)(?:$|/) { return 404; }
  location ~ ^/(?:\.|autotest|occ|issue|indie|db_|console) { return 404; }

  location ~ ^/(?:composer\.(?:json|lock)|package(?:-lock)?\.json|core/shipped\.json)$ {
    return 404;
  }

  rewrite ^/(?!index|remote|public|cron|status|ocs\/v[12]|ocs-provider\/.+|core\/ajax\/update|updater\/.+|.+\/richdocumentscode(_arm64)?\/proxy) /index.php\$request_uri;

  fastcgi_split_path_info ^(.+?\.php)(/.*)$;
  set \$path_info \$fastcgi_path_info;

  try_files \$fastcgi_script_name =404;

  include fastcgi_params;
  fastcgi_pass php-handler;

  fastcgi_param SCRIPT_FILENAME \$document_root\$fastcgi_script_name;
  fastcgi_param PATH_INFO \$path_info;
  fastcgi_param HTTPS on;
  fastcgi_param modHeadersAvailable true;
  fastcgi_param front_controller_active true;
  fastcgi_max_temp_file_size 0;

  location ~ \.(?:css|js|mjs|svg|gif|ico|jpg|png|webp|wasm|tflite|map|ogg|flac|mp4|webm)$ {
    try_files \$uri /index.php\$request_uri;
    add_header Cache-Control "public, max-age=15778463\$asset_immutable" always;
    add_header Referrer-Policy "no-referrer" always;
    add_header X-Content-Type-Options "nosniff" always;
    add_header X-Frame-Options "SAMEORIGIN" always;
    add_header X-Permitted-Cross-Domain-Policies "none" always;
    add_header X-Robots-Tag "noindex, nofollow" always;
    access_log off;
  }

  location ~ \.(otf|woff2?)$ {
    try_files \$uri /index.php\$request_uri;
    expires 7d;
    access_log off;
  }

  location /remote {
    return 301 /remote.php\$request_uri;
  }

  location / {
    try_files \$uri \$uri/ /index.php\$request_uri;
  }
}
EOF
            ln -sf "$avail_path" "$enabled_path"

            if ! sudo nginx -t; then
                handle_error "$avail_path" "$enabled_path"
            fi

            if ! sudo certbot --nginx -d "$domain_name"; then
                handle_certbot_error "$avail_path" "$enabled_path"
            else
                echo -e "SSL certificate sucessfully deployed! Visit \033[34mhttps://${domain_name}\033[0m in your browser.\n\nExiting now...\n"
                sudo nginx -t
                sudo systemctl reload nginx
            fi
            exit 0
        fi

        if [ "$presets" = "3" ]; then
            read -p "Enter domain name: " domain_name
            avail_path="/etc/nginx/sites-available/plex"
            enabled_path="/etc/nginx/sites-enabled/plex"

            cat <<EOF > "$avail_path"
server {
  listen 80;
  server_name $domain_name;

  location / {
    proxy_pass http://127.0.0.1:3400;
    proxy_http_version 1.1;
    proxy_set_header Upgrade \$http_upgrade;
    proxy_set_header Connection 'upgrade';
    proxy_set_header Host \$host;
    proxy_cache_bypass \$http_upgrade;
    proxy_set_header X-Real-IP \$remote_addr;
  }

  error_page 502 /502.html;
  location = /502.html {
    root /home/PlexStore;
  }
}
EOF
            ln -sf "$avail_path" "$enabled_path"

            if ! sudo nginx -t; then
                handle_error "$avail_path" "$enabled_path"
            fi

            if ! sudo certbot --nginx -d "$domain_name"; then
                handle_certbot_error "$avail_path" "$enabled_path"
            fi

            sudo nginx -t
            sudo systemctl reload nginx
            echo -e "SSL certificate successfully deployed! Visit \033[0mhttps://${domain_name}\033[0m in you browser.\n"
            exit 0
        fi

        if [ "$presets" = "4" ]; then
            read -p "$(echo -e '\033[31mWARNING!!\033[0m Opening the auto config GUI to the public internet is risky. Do you wish to proceed? (y or n): ')" gui
            if [ "$gui" = "n" ]; then exit 0; fi

            read -p "Enter domain name: " domain_name
            rm -f /etc/nginx/sites-available/auto-config /etc/nginx/sites-enabled/auto-config

            avail_path="/etc/nginx/sites-available/auto-config"
            enabled_path="/etc/nginx/sites-enabled/auto-config"

            cat <<EOF > "$avail_path"
server {
  listen 80;
  listen [::]:80;

  server_name $domain_name;

  root /var/www/auto-config/;
  index index.html index.php;

  location / {
    autoindex on;
    try_files \$uri \$uri/ \$uri/index.html \$uri.html \$uri.php =404;
  }
}
EOF
            ln -sf "$avail_path" "$enabled_path"

            if ! sudo nginx -t; then
                handle_error "$avail_path" "$enabled_path"
            fi

            if ! sudo certbot --nginx -d "$domain_name"; then
                handle_certbot_error "$avail_path" "$enabled_path"
            else
                echo -e "SSL certificate sucessfully deployed! Visit \033[34mhttps://${domain_name}\033[0m in your browser.\n\nExiting now...\n"
                sudo nginx -t
                sudo systemctl reload nginx
            fi
            exit 0
        fi
    fi

    if [ "$preset" = "q" ]; then exit 0; fi

    read -p "Input 1 for reverse proxy or 2 for standalone directory: " type
    if [ "$type" = "1" ]; then
        read -p "Enter the proxy port: " proxy
    elif [ "$type" = "2" ]; then
        read -p "Enter directory path: " directory
    fi

    read -p "Enter name of the config: " config_name
    read -p "Enter domain name: " domain_name

    avail_path="/etc/nginx/sites-available/$config_name"
    enabled_path="/etc/nginx/sites-enabled/$config_name"

    if [ "$type" = "1" ]; then
        cat <<EOF > "$avail_path"
server {
  server_name $domain_name;

  location / {
    proxy_pass http://127.0.0.1:$proxy;
    proxy_set_header Host \$host;
    proxy_set_header X-Real-IP \$remote_addr;
    proxy_set_header X-Forwarded-For \$proxy_add_x_forwarded_for;
    proxy_set_header X-Forwarded-Proto \$scheme;
    proxy_set_header X-Forwarded-Protocol \$scheme;
    proxy_set_header X-Forwarded-Host \$http_host;
    proxy_http_version 1.1;
    proxy_set_header Upgrade \$http_upgrade;
    proxy_set_header Connection "upgrade";
  }
}
EOF
    elif [ "$type" = "2" ]; then
        if php -v >/dev/null 2>&1; then
            cat <<EOF > "$avail_path"
server {
  listen 80;
  listen [::]:80;

  server_name $domain_name;

  root $directory;
  index index.html index.htm index.php;

  location / {
    autoindex on;
    try_files \$uri \$uri/ \$uri/index.html \$uri.html \$uri.php =404;
  }
}
EOF
        else
            cat <<EOF > "$avail_path"
server {
  listen 80;
  listen [::]:80;

  server_name $domain_name;

  root $directory;
  index index.html index.htm;

  location / {
    autoindex on;
    try_files \$uri \$uri/ \$uri/index.html \$uri.html =404;
  }
}
EOF
        fi
    fi

    ln -sf "$avail_path" "$enabled_path"

    if ! sudo nginx -t; then
        handle_error "$avail_path" "$enabled_path"
    fi

    if ! sudo certbot --nginx -d "$domain_name"; then
        handle_certbot_error "$avail_path" "$enabled_path"
    else
        echo -e "SSL certificate sucessfully deployed! Visit \033[34mhttps://${domain_name}\033[0m in your browser.\n\nExiting now...\n"
        sudo nginx -t
        sudo systemctl reload nginx
    fi
}

main "$@"
