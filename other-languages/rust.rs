use std::fs::{self, File};
use std::io::{self, BufRead, Write};
use std::os::unix::fs::symlink;
use std::process::{Command, Stdio};

const CURRENT_VERSION: &str = "v2.0.0";
const REPO_URL: &str = "https://api.github.com/repos/sizablesplash86-hub/nginx-auto-config/releases/latest";

fn root_check() -> i32 {
    unsafe {
        if libc::geteuid() != 0 {
            println!("\n\x1b[31mEnter root first\x1b[0m\n");
            return 1;
        }
    }
    0
}

fn update_check() {
    let command = format!(
        "curl -s -H \"User-Agent: auto-config-app\" {} | grep '\"tag_name\":' | sed -E 's/.*\"([^\"]+)\".*/\\1/'",
        REPO_URL
    );
    let output = Command::new("sh").arg("-c").arg(command).output();

    if let Ok(out) = output {
        let latest_version = String::from_utf8_lossy(&out.stdout).trim().to_string();
        if !latest_version.is_empty() && latest_version != CURRENT_VERSION {
            print!("{} --> {}", CURRENT_VERSION, latest_version);
            print!("Would you like to upgrade? (y or n): ");
            io::stdout().flush().unwrap();

            let mut ans = String::new();
            io::stdin().read_line(&mut ans).unwrap();
            if ans.trim() == "y" {
                println!("\nUpgrading to {}...\n", latest_version);
                let ver_num = if latest_version.starts_with('v') {
                    &latest_version[1..]
                } else {
                    &latest_version
                };

                let update_cmd = format!(
                    "curl -sfL https://github.com/sizablesplash86-hub/nginx-auto-config/releases/download/{}/auto-config_{}_amd64.deb -o /tmp/auto-config_update.deb && dpkg -i /tmp/auto-config_update.deb || apt-get install -f -y && rm -f /tmp/auto-config_update.deb",
                    latest_version, ver_num
                );

                let status = Command::new("sh").arg("-c").arg(update_cmd).status();
                if let Ok(st) = status {
                    if st.success() {
                        println!("\nUpgrade to {} completed successfully!\n", latest_version);
                        let _ = Command::new("auto-config").status();
                        std::process::exit(0);
                    }
                }
                println!("\nUPGRADE FALIED! Continuing with current version...\n");
            }
        }
    }
}

fn get_lan_ip() -> String {
    let output = Command::new("hostname").arg("-I").output();
    if let Ok(out) = output {
        let ips = String::from_utf8_lossy(&out.stdout);
        if let Some(ip) = ips.split_whitespace().next() {
            return ip.to_string();
        }
    }
    "127.0.0.1".to_string()
}

fn handle_error_and_rollback(avail: &str, enabled: &str) {
    println!("\x1b[31mUNKNOWN ERROR OCCURRED\x1b[0m");
    println!("Removing broken configuration...");
    let _ = fs::remove_file(avail);
    let _ = fs::remove_file(enabled);
    let _ = Command::new("sudo").arg("nginx").arg("-t").status();
}

fn handle_certbot_failure(avail: &str, enabled: &str) {
    println!("\x1b[31mERROR\x1b[0m SSL certificate failed to deploy\n");
    print!("Would you like to remove config in /etc/nginx/sites-available /etc/nginx/sites-enabled? (y or n): ");
    io::stdout().flush().unwrap();

    let mut rem = String::new();
    io::stdin().read_line(&mut rem).unwrap();
    let rem = rem.trim();

    if rem == "y" {
        println!("Removing broken configuration...");
        let _ = fs::remove_file(avail);
        let _ = fs::remove_file(enabled);
        let _ = Command::new("sudo").arg("nginx").arg("-t").status();
        std::process::exit(0);
    } else if rem == "n" || rem == "q" {
        println!("Exiting now...\n");
        std::process::exit(0);
    }
}

fn input_prompt(prompt: &str) -> String {
    print!("{}", prompt);
    io::stdout().flush().unwrap();
    let mut buffer = String::new();
    io::stdin().read_line(&mut buffer).unwrap();
    buffer.trim().to_string()
}

fn main() {
    if root_check() != 0 {
        return;
    }

    update_check();
    let lan_ip = get_lan_ip();

    println!("\nWelcome to the NGINX Auto Config {}! Now known as N.A.P. for NGINX Auto Program because it's as easy as taking a NAP.\n", CURRENT_VERSION);
    println!("Visit \x1b[34mhttp://{}:3487\x1b[0m in the browser to use the graphical interface\n", lan_ip);
    println!("This program is designed to work with my server guide \x1b[34mhttps://www.sizablesplash.com/server-guide\x1b[0m\n");
    println!("at any prompt, input q to exit\n");

    let preset = input_prompt("Would you like to pick between presets? (y or n): ");

    if preset == "y" {
        let presets = input_prompt("Input 1 for Jellyfin, 2 for Nextcloud LEMP stacks, 3 for Plex, 4 for auto config GUI (NOT RECOMMENDED): ");

        if presets == "1" {
            let domain_name = input_prompt("Enter domain name: ");
            let avail_path = "/etc/nginx/sites-available/jellyfin";
            let enabled_path = "/etc/nginx/sites-enabled/jellyfin";

            let config = format!(
"server {{
  server_name {};

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
    proxy_set_header Connection \"upgrade\";
  }}
}}
", domain_name);

            fs::write(avail_path, config).unwrap();
            let _ = symlink(avail_path, enabled_path);

            if Command::new("sudo").args(["nginx", "-t"]).status().map_or(true, |s| !s.success()) {
                handle_error_and_rollback(avail_path, enabled_path);
                return;
            }

            if Command::new("sudo").args(["certbot", "--nginx", "-d", &domain_name]).status().map_or(true, |s| !s.success()) {
                handle_certbot_failure(avail_path, enabled_path);
            }

            let _ = Command::new("sudo").args(["nginx", "-t"]).status();
            let _ = Command::new("sudo").args(["systemctl", "reload", "nginx"]).status();
            println!("SSL certificate sucessfully deployed! Visit \x1b[34mhttps://{}\x1b[0m in your browser.\n\nExiting now...\n", domain_name);
            return;
        }

        if presets == "2" {
            println!("Make sure to follow my LEMP Stacks guide at \x1b[34mhttps://www.sizablesplash.com/server-guide/prereqs/OS-config/nextcloud/LEMP-stacks\x1b[0m\n");
            
            let output = Command::new("sh").arg("-c").arg("php -r 'echo PHP_MAJOR_VERSION.\".\".PHP_MINOR_VERSION;' 2>/dev/null").output();
            let php_ver = output.map_or(String::new(), |out| String::from_utf8_lossy(&out.stdout).trim().to_string());

            if php_ver.is_empty() {
                println!("\x1b[31mPHP not installed.\x1b[0m Exiting now...\n");
                return;
            }

            let php_sock = format!("/run/php/php{}-fpm.sock", php_ver);
            let domain_name = input_prompt("Enter domain name: ");

            let avail_path = "/etc/nginx/sites-available/nextcloud";
            let enabled_path = "/etc/nginx/sites-enabled/nextcloud";

            let config = format!(
"upstream php-handler {{
  server unix:{};
}}

map $arg_v $asset_immutable {{
  \"\" \"\";
  default \", immutable\";
}}

server {{
  listen 80;
  listen [::]:80;
  server_name {};
  server_tokens off;
  return 301 https://$server_name$request_uri;
}}

server {{
  http2 on;
  server_name {};
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

  add_header Referrer-Policy \"no-referrer\" always;
  add_header X-Content-Type-Options \"nosniff\" always;
  add_header X-Frame-Options \"SAMEORIGIN\" always;
  add_header X-Permitted-Cross-Domain-Policies \"none\" always;
  add_header X-Robots-Tag \"noindex, nofollow\" always;
  add_header Permissions-Policy \"camera=(), microphone=(), geolocation=()\" always;
  add_header Content-Security-Policy \"default-src 'self'; script-src 'self' 'unsafe-inline' 'unsafe-eval'; style-src 'self' 'unsafe-inline'; img-src 'self' data: blob:; font-src 'self'; connect-src 'self'; frame-ancestors 'self'\" always;

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
    add_header Cache-Control \"public, max-age=15778463$asset_immutable\" always;
    add_header Referrer-Policy \"no-referrer\" always;
    add_header X-Content-Type-Options \"nosniff\" always;
    add_header X-Frame-Options \"SAMEORIGIN\" always;
    add_header X-Permitted-Cross-Domain-Policies \"none\" always;
    add_header X-Robots-Tag \"noindex, nofollow\" always;
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
", php_sock, domain_name, domain_name);

            fs::write(avail_path, config).unwrap();
            let _ = symlink(avail_path, enabled_path);

            if Command::new("sudo").args(["nginx", "-t"]).status().map_or(true, |s| !s.success()) {
                handle_error_and_rollback(avail_path, enabled_path);
                return;
            }

            if Command::new("sudo").args(["certbot", "--nginx", "-d", &domain_name]).status().map_or(true, |s| !s.success()) {
                handle_certbot_failure(avail_path, enabled_path);
            } else {
                println!("SSL certificate sucessfully deployed! Visit \x1b[34mhttps://{}\x1b[0m in your browser.\n\nExiting now...\n", domain_name);
                let _ = Command::new("sudo").args(["nginx", "-t"]).status();
                let _ = Command::new("sudo").args(["systemctl", "reload", "nginx"]).status();
            }
            return;
        }

        if presets == "3" {
            let domain_name = input_prompt("Enter domain name: ");
            let avail_path = "/etc/nginx/sites-available/plex";
            let enabled_path = "/etc/nginx/sites-enabled/plex";

            let config = format!(
"server {{
  listen 80;
  server_name {};

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
", domain_name);

            fs::write(avail_path, config).unwrap();
            let _ = symlink(avail_path, enabled_path);

            if Command::new("sudo").args(["nginx", "-t"]).status().map_or(true, |s| !s.success()) {
                handle_error_and_rollback(avail_path, enabled_path);
                return;
            }

            if Command::new("sudo").args(["certbot", "--nginx", "-d", &domain_name]).status().map_or(true, |s| !s.success()) {
                handle_certbot_failure(avail_path, enabled_path);
            }

            let _ = Command::new("sudo").args(["nginx", "-t"]).status();
            let _ = Command::new("sudo").args(["systemctl", "reload", "nginx"]).status();
            println!("SSL certificate successfully deployed! Visit \x1b[0mhttps://{}\x1b[0m in you browser.\n", domain_name);
            return;
        }

        if presets == "4" {
            let gui = input_prompt("\x1b[31mWARNING!!\x1b[0m Opening the auto config GUI to the public internet is risky. Do you wish to proceed? (y or n): ");
            if gui == "n" { return; }

            let domain_name = input_prompt("Enter domain name: ");
            let _ = Command::new("sh").arg("-c").arg("rm -f /etc/nginx/sites-available/auto-config /etc/nginx/sites-enabled/auto-config").status();

            let avail_path = "/etc/nginx/sites-available/auto-config";
            let enabled_path = "/etc/nginx/sites-enabled/auto-config";

            let config = format!(
"server {{
  listen 80;
  listen [::]:80;

  server_name {};

  root /var/www/auto-config/;
  index index.html index.php;

  location / {{
    autoindex on;
    try_files $uri $uri/ $uri/index.html $uri.html $uri.php =404;
  }}
}}
", domain_name);

            fs::write(avail_path, config).unwrap();
            let _ = symlink(avail_path, enabled_path);

            if Command::new("sudo").args(["nginx", "-t"]).status().map_or(true, |s| !s.success()) {
                handle_error_and_rollback(avail_path, enabled_path);
                return;
            }

            if Command::new("sudo").args(["certbot", "--nginx", "-d", &domain_name]).status().map_or(true, |s| !s.success()) {
                handle_certbot_failure(avail_path, enabled_path);
            } else {
                println!("SSL certificate sucessfully deployed! Visit \x1b[34mhttps://{}\x1b[0m in your browser.\n\nExiting now...\n", domain_name);
                let _ = Command::new("sudo").args(["nginx", "-t"]).status();
                let _ = Command::new("sudo").args(["systemctl", "reload", "nginx"]).status();
            }
            return;
        }
    }

    if preset == "q" { return; }

    let config_type = input_prompt("Input 1 for reverse proxy or 2 for standalone directory: ");
    let mut proxy = String::new();
    let mut directory = String::new();

    if config_type == "1" {
        proxy = input_prompt("Enter the proxy port: ");
    } else if config_type == "2" {
        directory = input_prompt("Enter directory path: ");
    }

    let config_name = input_prompt("Enter name of the config: ");
    let domain_name = input_prompt("Enter domain name: ");

    let avail_path = format!("/etc/nginx/sites-available/{}", config_name);
    let enabled_path = format!("/etc/nginx/sites-enabled/{}", config_name);

    let config = if config_type == "1" {
        format!(
"server {{
  server_name {};

  location / {{
    proxy_pass http://127.0.0.1:{};
    proxy_set_header Host $host;
    proxy_set_header X-Real-IP $remote_addr;
    proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
    proxy_set_header X-Forwarded-Proto $scheme;
    proxy_set_header X-Forwarded-Protocol $scheme;
    proxy_set_header X-Forwarded-Host $http_host;
    proxy_http_version 1.1;
    proxy_set_header Upgrade $http_upgrade;
    proxy_set_header Connection \"upgrade\";
  }}
}}
", domain_name, proxy)
    } else {
        let has_php = Command::new("sh").arg("-c").arg("php -v 1>/dev/null 2>&1").status().map_or(false, |s| s.success());
        if has_php {
            format!(
"server {{
  listen 80;
  listen [::]:80;

  server_name {};

  root {};
  index index.html index.htm index.php;

  location / {{
    autoindex on;
    try_files $uri $uri/ $uri/index.html $uri.html $uri.php =404;
  }}
}}
", domain_name, directory)
        } else {
            format!(
"server {{
  listen 80;
  listen [::]:80;

  server_name {};

  root {};
  index index.html index.htm;

  location / {{
    autoindex on;
    try_files $uri $uri/ $uri/index.html $uri.html =404;
  }}
}}
", domain_name, directory)
        }
    };

    fs::write(&avail_path, config).unwrap();
    let _ = symlink(&avail_path, &enabled_path);

    if Command::new("sudo").args(["nginx", "-t"]).status().map_or(true, |s| !s.success()) {
        handle_error_and_rollback(&avail_path, &enabled_path);
        return;
    }

    if Command::new("sudo").args(["certbot", "--nginx", "-d", &domain_name]).status().map_or(true, |s| !s.success()) {
        handle_certbot_failure(&avail_path, &enabled_path);
    } else {
        println!("SSL certificate sucessfully deployed! Visit \x1b[34mhttps://{}\x1b[0m in your browser.\n\nExiting now...\n", domain_name);
        let _ = Command::new("sudo").args(["nginx", "-t"]).status();
        let _ = Command::new("sudo").args(["systemctl", "reload", "nginx"]).status();
    }
}
