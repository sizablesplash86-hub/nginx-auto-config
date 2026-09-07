#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define CURRENT_VERSION "v2.0.0"
#define REPO_URL "https://api.github.com/repos/sizablesplash86-hub/nginx-auto-config/releases/latest"

int root_check() {
    if (geteuid() != 0) {
        std::cout << "\n\033[31mEnter root first\033[0m\n\n";
        return 1;
    }
    return 0;
}

void update_check() {
    char command[512];
    char latest_version[64] = {0};

    snprintf(command, sizeof(command), "curl -s -H \"User-Agent: auto-config-app\" %s | grep '\"tag_name\":' | sed -E 's/.*\"([^\"]+)\".*/\\1/'", REPO_URL);

    FILE *fp = popen(command, "r");
    if (fp == nullptr) return;

    if (fgets(latest_version, sizeof(latest_version), fp) != nullptr) {
        latest_version[strcspn(latest_version, "\r\n")] = '\0';
    }
    pclose(fp);

    if (strlen(latest_version) > 0 && strcmp(latest_version, CURRENT_VERSION) != 0) {
        std::cout << CURRENT_VERSION << " --> " << latest_version;
        std::cout << "Would you like to upgrade? (y or n): ";

        char ans;
        std::cin >> ans;

        if (ans == 'y') {
            std::cout << "\nUpgrading to " << latest_version << "...\n\n";

            const char *ver_num = (latest_version[0] == 'v') ? latest_version + 1 : latest_version;
            char update_cmd[1024];
            snprintf(update_cmd, sizeof(update_cmd),
                "curl -sfL https://github.com/sizablesplash86-hub/nginx-auto-config/releases/download/%s/auto-config_%s_amd64.deb -o /tmp/auto-config_update.deb && "
                "dpkg -i /tmp/auto-config_update.deb || apt-get install -f -y && "
                "rm -f /tmp/auto-config_update.deb",
                latest_version, ver_num
            );

            int res = system(update_cmd);
            if (res == 0) {
                std::cout << "\nUpgrade to " << latest_version << " completed successfully!\n\n";
                system("auto-config");
                exit(0);
            } else {
                std::cout << "\nUPGRADE FALIED! Continuing with current version...\n\n";
            }
        }
    }
}

void handle_error_and_rollback(const std::string& avail_path, const std::string& enabled_path) {
    std::cout << "\033[31mUNKNOWN ERROR OCCURRED\033[0m\n";
    std::cout << "Removing broken configuration...\n";
    unlink(avail_path.c_str());
    unlink(enabled_path.c_str());
    system("sudo nginx -t");
}

void handle_certbot_failure(const std::string& avail_path, const std::string& enabled_path) {
    char rem;
    std::cout << "\033[31mERROR\033[0m SSL certificate failed to deploy\n\n";
    std::cout << "Would you like to remove config in /etc/nginx/sites-available /etc/nginx/sites-enabled? (y or n): ";
    std::cin >> rem;
    if (rem == 'y') {
        std::cout << "Removing broken configuration...\n";
        unlink(avail_path.c_str());
        unlink(enabled_path.c_str());
        system("sudo nginx -t");
        exit(0);
    }
    if (rem == 'n' || rem == 'q') {
        std::cout << "Exiting now...\n\n";
        exit(0);
    }
}

int main() {
    if (root_check() != 0) return 0;

    update_check();

    struct ifaddrs *ifaddr, *ifa;
    char lan_ip[INET_ADDRSTRLEN] = "127.0.0.1";

    if (getifaddrs(&ifaddr) == 0) {
        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr == NULL || ifa->ifa_addr->sa_family != AF_INET) continue;
            if (strcmp(ifa->ifa_name, "lo") == 0 || strncmp(ifa->ifa_name, "wg", 2) == 0 || strncmp(ifa->ifa_name, "tailscale", 9) == 0 || strncmp(ifa->ifa_name, "docker", 6) == 0 || strncmp(ifa->ifa_name, "veth", 4) == 0) continue;

            struct sockaddr_in *pAddr = (struct sockaddr_in *)ifa->ifa_addr;
            inet_ntop(AF_INET, &pAddr->sin_addr, lan_ip, sizeof(lan_ip));
            break;
        }
        freeifaddrs(ifaddr);
    }

    std::cout << "\nWelcome to the NGINX Auto Config " << CURRENT_VERSION << "! Now known as N.A.P. for NGINX Auto Program because it's as easy as taking a NAP.\n\n";
    std::cout << "Visit \033[34mhttp://" << lan_ip << ":3487\033[0m in the browser to use the graphical interface\n\n";
    std::cout << "This program is designed to work with my server guide \033[34mhttps://www.sizablesplash.com/server-guide\033[0m\n\n";
    std::cout << "at any prompt, input q to exit\n\n";

    char preset;
    std::cout << "Would you like to pick between presets? (y or n): ";
    std::cin >> preset;

    if (preset == 'y') {
        char presets;
        std::cout << "Input 1 for Jellyfin, 2 for Nextcloud LEMP stacks, 3 for Plex, 4 for auto config GUI (NOT RECOMMENDED): ";
        std::cin >> presets;

        std::string domain_name;

        if (presets == '1') {
            std::cout << "Enter domain name: ";
            std::cin >> domain_name;

            std::string avail_path = "/etc/nginx/sites-available/jellyfin";
            std::ofstream fp(avail_path);
            fp << "server {\n"
               << "  server_name " << domain_name << ";\n\n"
               << "  location / {\n"
               << "    proxy_pass http://127.0.0.1:8096;\n"
               << "    proxy_set_header Host $host;\n"
               << "    proxy_set_header X-Real-IP $remote_addr;\n"
               << "    proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;\n"
               << "    proxy_set_header X-Forwarded-Proto $scheme;\n"
               << "    proxy_set_header X-Forwarded-Protocol $scheme;\n"
               << "    proxy_set_header X-Forwarded-Host $http_host;\n"
               << "    proxy_http_version 1.1;\n"
               << "    proxy_set_header Upgrade $http_upgrade;\n"
               << "    proxy_set_header Connection \"upgrade\";\n"
               << "  }\n"
               << "}\n";
            fp.close();

            std::string enabled_path = "/etc/nginx/sites-enabled/jellyfin";
            symlink(avail_path.c_str(), enabled_path.c_str());

            if (system("sudo nginx -t") != 0) {
                handle_error_and_rollback(avail_path, enabled_path);
                return 0;
            }

            std::string certbot_cmd = "sudo certbot --nginx -d " + domain_name;
            if (system(certbot_cmd.c_str()) != 0) {
                handle_certbot_failure(avail_path, enabled_path);
            }

            system("sudo nginx -t");
            system("sudo systemctl reload nginx");
            std::cout << "SSL certificate sucessfully deployed! Visit \033[34mhttps://" << domain_name << "\033[0m in your browser.\n\nExiting now...\n\n";
            return 0;
        }

        if (presets == '2') {
            std::cout << "Make sure to follow my LEMP Stacks guide at \033[34mhttps://www.sizablesplash.com/server-guide/prereqs/OS-config/nextcloud/LEMP-stacks\033[0m\n\n";

            char php_ver[256] = {0};
            FILE *cmd = popen("php -r 'echo PHP_MAJOR_VERSION.\".\".PHP_MINOR_VERSION;' 2>/dev/null", "r");

            if (cmd == nullptr || fgets(php_ver, sizeof(php_ver), cmd) == nullptr || strlen(php_ver) == 0) {
                if (cmd != nullptr) pclose(cmd);
                std::cout << "\033[31mPHP not installed.\033[0m Exiting now...\n\n";
                return 0;
            }
            pclose(cmd);
            php_ver[strcspn(php_ver, "\r\n")] = 0;

            std::string php_sock = std::string("/run/php/php") + php_ver + "-fpm.sock";

            std::cout << "Enter domain name: ";
            std::cin >> domain_name;

            std::string avail_path = "/etc/nginx/sites-available/nextcloud";
            std::ofstream fp(avail_path);
            fp << "upstream php-handler {\n"
               << "  server unix:" << php_sock << ";\n"
               << "}\n\n"
               << "map $arg_v $asset_immutable {\n"
               << "  \"\" \"\";\n"
               << "  default \", immutable\";\n"
               << "}\n\n"
               << "server {\n"
               << "  listen 80;\n"
               << "  listen [::]:80;\n"
               << "  server_name " << domain_name << ";\n"
               << "  server_tokens off;\n"
               << "  return 301 https://$server_name$request_uri;\n"
               << "}\n\n"
               << "server {\n"
               << "  http2 on;\n"
               << "  server_name " << domain_name << ";\n"
               << "  root /var/www/nextcloud;\n"
               << "  server_tokens off;\n\n"
               << "  client_max_body_size 512M;\n"
               << "  client_body_timeout 300s;\n"
               << "  fastcgi_buffers 64 4K;\n\n"
               << "  gzip on;\n"
               << "  gzip_vary on;\n"
               << "  gzip_comp_level 4;\n"
               << "  gzip_min_length 256;\n"
               << "  gzip_proxied expired no-cache no-store private no_last_modified no_etag auth;\n"
               << "  gzip_types application/atom+xml text/javascript application/javascript application/json application/ld+json application/manifest+json application/rss+xml application/vnd.geo+json application/vnd.ms-fontobject application/wasm application/x-font-ttf application/x-web-app-manifest+json application/xhtml+xml application/xml font/opentype image/bmp image/svg+xml image/x-icon text/cache-manifest text/css text/plain text/vcard text/vnd.rim.location.xloc text/vtt text/x-component text/x-cross-domain-policy;\n\n"
               << "  client_body_buffer_size 512k;\n\n"
               << "  add_header Referrer-Policy \"no-referrer\" always;\n"
               << "  add_header X-Content-Type-Options \"nosniff\" always;\n"
               << "  add_header X-Frame-Options \"SAMEORIGIN\" always;\n"
               << "  add_header X-Permitted-Cross-Domain-Policies \"none\" always;\n"
               << "  add_header X-Robots-Tag \"noindex, nofollow\" always;\n"
               << "  add_header Permissions-Policy \"camera=(), microphone=(), geolocation=()\" always;\n"
               << "  add_header Content-Security-Policy \"default-src 'self'; script-src 'self' 'unsafe-inline' 'unsafe-eval'; style-src 'self' 'unsafe-inline'; img-src 'self' data: blob:; font-src 'self'; connect-src 'self'; frame-ancestors 'self'\" always;\n\n"
               << "  fastcgi_hide_header X-Powered-By;\n\n"
               << "  include mime.types;\n"
               << "  types {\n"
               << "    text/javascript mjs;\n"
               << "    application/wasm wasm;\n"
               << "  }\n\n"
               << "  index index.php index.html /index.php$request_uri;\n\n"
               << "  location = / {\n"
               << "    if ( $http_user_agent ~ ^DavClnt ) {\n"
               << "      return 302 /remote.php/webdav/$is_args$args;\n"
               << "    }\n"
               << "  }\n\n"
               << "  location = /robots.txt {\n"
               << "    allow all;\n"
               << "    log_not_found off;\n"
               << "    access_log off;\n"
               << "  }\n\n"
               << "  location ^~ /.well-known {\n"
               << "    location = /.well-known/carddav { return 301 /remote.php/dav/; }\n"
               << "    location = /.well-known/caldav { return 301 /remote.php/dav/; }\n"
               << "    location /.well-known/acme-challenge { try_files $uri $uri/ =404; }\n"
               << "    location /.well-known/pki-validation { try_files $uri $uri/ =404; }\n"
               << "    return 301 /index.php$request_uri;\n"
               << "  }\n\n"
               << "  location ~ ^/(?:build|tests|config|lib|3rdparty|templates|data)(?:$|/) { return 404; }\n"
               << "  location ~ ^/(?:\\.|autotest|occ|issue|indie|db_|console) { return 404; }\n\n"
               << "  location ~ ^/(?:composer\\.(?:json|lock)|package(?:-lock)?\\.json|core/shipped\\.json)$ {\n"
               << "    return 404;\n"
               << "  }\n\n"
               << "  rewrite ^/(?!index|remote|public|cron|status|ocs\\/v[12]|ocs-provider\\/.+|core\\/ajax\\/update|updater\\/.+|.+\\/richdocumentscode(_arm64)?\\/proxy) /index.php$request_uri;\n\n"
               << "  fastcgi_split_path_info ^(.+?\\.php)(/.*)$;\n"
               << "  set $path_info $fastcgi_path_info;\n\n"
               << "  try_files $fastcgi_script_name =404;\n\n"
               << "  include fastcgi_params;\n"
               << "  fastcgi_pass php-handler;\n\n"
               << "  fastcgi_param SCRIPT_FILENAME $document_root$fastcgi_script_name;\n"
               << "  fastcgi_param PATH_INFO $path_info;\n"
               << "  fastcgi_param HTTPS on;\n"
               << "  fastcgi_param modHeadersAvailable true;\n"
               << "  fastcgi_param front_controller_active true;\n"
               << "  fastcgi_max_temp_file_size 0;\n\n"
               << "  location ~ \\.(?:css|js|mjs|svg|gif|ico|jpg|png|webp|wasm|tflite|map|ogg|flac|mp4|webm)$ {\n"
               << "    try_files $uri /index.php$request_uri;\n"
               << "    add_header Cache-Control \"public, max-age=15778463$asset_immutable\" always;\n"
               << "    add_header Referrer-Policy \"no-referrer\" always;\n"
               << "    add_header X-Content-Type-Options \"nosniff\" always;\n"
               << "    add_header X-Frame-Options \"SAMEORIGIN\" always;\n"
               << "    add_header X-Permitted-Cross-Domain-Policies \"none\" always;\n"
               << "    add_header X-Robots-Tag \"noindex, nofollow\" always;\n"
               << "    access_log off;\n"
               << "  }\n\n"
               << "  location ~ \\.(otf|woff2?)$ {\n"
               << "    try_files $uri /index.php$request_uri;\n"
               << "    expires 7d;\n"
               << "    access_log off;\n"
               << "  }\n\n"
               << "  location /remote {\n"
               << "    return 301 /remote.php$request_uri;\n"
               << "  }\n\n"
               << "  location / {\n"
               << "    try_files $uri $uri/ /index.php$request_uri;\n"
               << "  }\n"
               << "}\n";
            fp.close();

            std::string enabled_path = "/etc/nginx/sites-enabled/nextcloud";
            symlink(avail_path.c_str(), enabled_path.c_str());

            if (system("sudo nginx -t") != 0) {
                handle_error_and_rollback(avail_path, enabled_path);
                return 0;
            }

            std::string certbot_cmd = "sudo certbot --nginx -d " + domain_name;
            if (system(certbot_cmd.c_str()) != 0) {
                handle_certbot_failure(avail_path, enabled_path);
            } else {
                std::cout << "SSL certificate sucessfully deployed! Visit \033[34mhttps://" << domain_name << "\033[0m in your browser.\n\nExiting now...\n\n";
                system("sudo nginx -t");
                system("sudo systemctl reload nginx");
            }
            return 0;
        }

        if (presets == '3') {
            std::cout << "Enter domain name: ";
            std::cin >> domain_name;

            std::string avail_path = "/etc/nginx/sites-available/plex";
            std::ofstream fp(avail_path);
            fp << "server {\n"
               << "  listen 80;\n"
               << "  server_name " << domain_name << ";\n\n"
               << "  location / {\n"
               << "    proxy_pass http://127.0.0.1:3400;\n"
               << "    proxy_http_version 1.1;\n"
               << "    proxy_set_header Upgrade $http_upgrade;\n"
               << "    proxy_set_header Connection 'upgrade';\n"
               << "    proxy_set_header Host $host;\n"
               << "    proxy_cache_bypass $http_upgrade;\n"
               << "    proxy_set_header X-Real-IP $remote_addr;\n"
               << "  }\n\n"
               << "  error_page 502 /502.html;\n"
               << "  location = /502.html {\n"
               << "    root /home/PlexStore;\n"
               << "  }\n"
               << "}\n";
            fp.close();

            std::string enabled_path = "/etc/nginx/sites-enabled/plex";
            symlink(avail_path.c_str(), enabled_path.c_str());

            if (system("sudo nginx -t") != 0) {
                handle_error_and_rollback(avail_path, enabled_path);
                return 0;
            }

            std::string certbot_cmd = "sudo certbot --nginx -d " + domain_name;
            if (system(certbot_cmd.c_str()) != 0) {
                handle_certbot_failure(avail_path, enabled_path);
            }

            system("sudo nginx -t");
            system("sudo systemctl reload nginx");
            std::cout << "SSL certificate successfully deployed! Visit \033[0mhttps://" << domain_name << "\033[0m in you browser.\n\n";
            return 0;
        }

        if (presets == '4') {
            char gui;
            std::cout << "\033[31mWARNING!!\033[0m Opening the auto config GUI to the public internet is risky. Do you wish to proceed? (y or n): ";
            std::cin >> gui;
            if (gui == 'n') return 0;

            std::cout << "Enter domain name: ";
            std::cin >> domain_name;

            system("rm -f /etc/nginx/sites-available/auto-config /etc/nginx/sites-enabled/auto-config");

            std::string avail_path = "/etc/nginx/sites-available/auto-config";
            std::ofstream fp(avail_path);
            fp << "server {\n"
               << "  listen 80;\n"
               << "  listen [::]:80;\n\n"
               << "  server_name " << domain_name << ";\n\n"
               << "  root /var/www/auto-config/;\n"
               << "  index index.html index.php;\n\n"
               << "  location / {\n"
               << "    autoindex on;\n"
               << "    try_files $uri $uri/ $uri/index.html $uri.html $uri.php =404;\n"
               << "  }\n"
               << "}\n";
            fp.close();

            std::string enabled_path = "/etc/nginx/sites-enabled/auto-config";
            symlink(avail_path.c_str(), enabled_path.c_str());

            if (system("sudo nginx -t") != 0) {
                handle_error_and_rollback(avail_path, enabled_path);
                return 0;
            }

            std::string certbot_cmd = "sudo certbot --nginx -d " + domain_name;
            if (system(certbot_cmd.c_str()) != 0) {
                handle_certbot_failure(avail_path, enabled_path);
            } else {
                std::cout << "SSL certificate sucessfully deployed! Visit \033[34mhttps://" << domain_name << "\033[0m in your browser.\n\nExiting now...\n\n";
                system("sudo nginx -t");
                system("sudo systemctl reload nginx");
            }
            return 0;
        }
        return 0;
    }

    if (preset == 'q') return 0;

    char type;
    std::cout << "Input 1 for reverse proxy or 2 for standalone directory: ";
    std::cin >> type;

    std::string proxy, directory, config_name, domain_name;

    if (type == '1') {
        std::cout << "Enter the proxy port: ";
        std::cin >> proxy;
    }
    if (type == '2') {
        std::cout << "Enter directory path: ";
        std::cin >> directory;
    }

    std::cout << "Enter name of the config: ";
    std::cin >> config_name;

    std::cout << "Enter domain name: ";
    std::cin >> domain_name;

    std::string avail_path = "/etc/nginx/sites-available/" + config_name;
    std::ofstream fp(avail_path);

    if (type == '1') {
        fp << "server {\n"
           << "  server_name " << domain_name << ";\n\n"
           << "  location / {\n"
           << "    proxy_pass http://127.0.0.1:" << proxy << ";\n"
           << "    proxy_set_header Host $host;\n"
           << "    proxy_set_header X-Real-IP $remote_addr;\n"
           << "    proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;\n"
           << "    proxy_set_header X-Forwarded-Proto $scheme;\n"
           << "    proxy_set_header X-Forwarded-Protocol $scheme;\n"
           << "    proxy_set_header X-Forwarded-Host $http_host;\n"
           << "    proxy_http_version 1.1;\n"
           << "    proxy_set_header Upgrade $http_upgrade;\n"
           << "    proxy_set_header Connection \"upgrade\";\n"
           << "  }\n"
           << "}\n";
    }

    if (type == '2') {
        if (system("php -v 1>/dev/null 2>&1") == 0) {
            fp << "server {\n"
               << "  listen 80;\n"
               << "  listen [::]:80;\n\n"
               << "  server_name " << domain_name << ";\n\n"
               << "  root " << directory << ";\n"
               << "  index index.html index.htm index.php;\n\n"
               << "  location / {\n"
               << "    autoindex on;\n"
               << "    try_files $uri $uri/ $uri/index.html $uri.html $uri.php =404;\n"
               << "  }\n"
               << "}\n";
        } else {
            fp << "server {\n"
               << "  listen 80;\n"
               << "  listen [::]:80;\n\n"
               << "  server_name " << domain_name << ";\n\n"
               << "  root " << directory << ";\n"
               << "  index index.html index.htm;\n\n"
               << "  location / {\n"
               << "    autoindex on;\n"
               << "    try_files $uri $uri/ $uri/index.html $uri.html =404;\n"
               << "  }\n"
               << "}\n";
        }
    }
    fp.close();

    std::string enabled_path = "/etc/nginx/sites-enabled/" + config_name;
    symlink(avail_path.c_str(), enabled_path.c_str());

    if (system("sudo nginx -t") != 0) {
        handle_error_and_rollback(avail_path, enabled_path);
        return 0;
    }

    std::string certbot_cmd = "sudo certbot --nginx -d " + domain_name;

    if (system(certbot_cmd.c_str()) != 0) {
        handle_certbot_failure(avail_path, enabled_path);
    } else {
        std::cout << "SSL certificate sucessfully deployed! Visit \033[34mhttps://" << domain_name << "\033[0m in your browser.\n\nExiting now...\n\n";
        system("sudo nginx -t");
        system("sudo systemctl reload nginx");
    }

    return 0;
}
