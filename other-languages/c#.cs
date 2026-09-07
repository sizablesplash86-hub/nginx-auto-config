using System;
using System.Diagnostics;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Text.Json;

class Program
{
    private const string CURRENT_VERSION = "v2.0.0";
    private const string REPO_URL = "https://api.github.com/repos/sizablesplash86-hub/nginx-auto-config/releases/latest";

    [DllImport("libc")]
    private static extern uint geteuid();

    [DllImport("libc")]
    private static extern int symlink(string target, string linkpath);

    private static int RootCheck()
    {
        if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux) && geteuid() != 0)
        {
            Console.WriteLine("\n\033[31mEnter root first\033[0m\n");
            return 1;
        }
        return 0;
    }

    private static void UpdateCheck()
    {
        try
        {
            var psi = new ProcessStartInfo
            {
                FileName = "sh",
                Arguments = $"-c \"curl -s -H \\\"User-Agent: auto-config-app\\\" {REPO_URL} | grep '\\\"tag_name\\\":' | sed -E 's/.*\\\"([^\\\"]+)\\\".*/\\\\1/'\"",
                RedirectStandardOutput = true,
                UseShellExecute = false
            };
            using var p = Process.Start(psi);
            string latestVersion = p.StandardOutput.ReadToEnd().Trim();

            if (!string.IsNullOrEmpty(latestVersion) && latestVersion != CURRENT_VERSION)
            {
                Console.Write($"{CURRENT_VERSION} --> {latestVersion}");
                Console.Write("Would you like to upgrade? (y or n): ");
                string ans = Console.ReadLine()?.Trim();

                if (ans == "y")
                {
                    Console.WriteLine($"\nUpgrading to {latestVersion}...\n");
                    string verNum = latestVersion.StartsWith("v") ? latestVersion[1..] : latestVersion;
                    string updateCmd = $"curl -sfL https://github.com/sizablesplash86-hub/nginx-auto-config/releases/download/{latestVersion}/auto-config_{verNum}_amd64.deb -o /tmp/auto-config_update.deb && " +
                                       "dpkg -i /tmp/auto-config_update.deb || apt-get install -f -y && " +
                                       "rm -f /tmp/auto-config_update.deb";

                    var updatePsi = new ProcessStartInfo("sh", $"-c \"{updateCmd}\"");
                    using var pUp = Process.Start(updatePsi);
                    pUp.WaitForExit();

                    if (pUp.ExitCode == 0)
                    {
                        Console.WriteLine($"\nUpgrade to {latestVersion} completed successfully!\n");
                        Process.Start("auto-config");
                        Environment.Exit(0);
                    }
                    else
                    {
                        Console.WriteLine("\nUPGRADE FALIED! Continuing with current version...\n");
                    }
                }
            }
        }
        catch { }
    }

    private static string GetLanIp()
    {
        try
        {
            using Socket socket = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, 0);
            socket.Connect("8.8.8.8", 65530);
            return ((IPEndPoint)socket.LocalEndPoint).Address.ToString();
        }
        catch
        {
            return "127.0.0.1";
        }
    }

    private static int RunCommand(string cmd)
    {
        var psi = new ProcessStartInfo("sh", $"-c \"{cmd}\"");
        using var p = Process.Start(psi);
        p.WaitForExit();
        return p.ExitCode;
    }

    private static void HandleErrorAndRollback(string availPath, string enabledPath)
    {
        Console.WriteLine("\033[31mUNKNOWN ERROR OCCURRED\033[0m");
        Console.WriteLine("Removing broken configuration...");
        if (File.Exists(availPath)) File.Delete(availPath);
        if (File.Exists(enabledPath)) File.Delete(enabledPath);
        RunCommand("sudo nginx -t");
    }

    private static void HandleCertbotFailure(string availPath, string enabledPath)
    {
        Console.WriteLine("\033[31mERROR\033[0m SSL certificate failed to deploy\n");
        Console.Write("Would you like to remove config in /etc/nginx/sites-available /etc/nginx/sites-enabled? (y or n): ");
        string rem = Console.ReadLine()?.Trim();
        if (rem == "y")
        {
            Console.WriteLine("Removing broken configuration...");
            if (File.Exists(availPath)) File.Delete(availPath);
            if (File.Exists(enabledPath)) File.Delete(enabledPath);
            RunCommand("sudo nginx -t");
            Environment.Exit(0);
        }
        else if (rem == "n" || rem == "q")
        {
            Console.WriteLine("Exiting now...\n");
            Environment.Exit(0);
        }
    }

    static void Main(string[] args)
    {
        if (RootCheck() != 0) return;

        UpdateCheck();
        string lanIp = GetLanIp();

        Console.WriteLine($"\nWelcome to the NGINX Auto Config {CURRENT_VERSION}! Now known as N.A.P. for NGINX Auto Program because it's as easy as taking a NAP.\n");
        Console.WriteLine($"Visit \033[34mhttp://{lanIp}:3487\033[0m in the browser to use the graphical interface\n");
        Console.WriteLine("This program is designed to work with my server guide \033[34mhttps://www.sizablesplash.com/server-guide\033[0m\n");
        Console.WriteLine("at any prompt, input q to exit\n");

        Console.Write("Would you like to pick between presets? (y or n): ");
        string preset = Console.ReadLine()?.Trim();

        if (preset == "y")
        {
            Console.Write("Input 1 for Jellyfin, 2 for Nextcloud LEMP stacks, 3 for Plex, 4 for auto config GUI (NOT RECOMMENDED): ");
            string presets = Console.ReadLine()?.Trim();

            if (presets == "1")
            {
                Console.Write("Enter domain name: ");
                string domainName = Console.ReadLine()?.Trim();
                string availPath = "/etc/nginx/sites-available/jellyfin";
                string enabledPath = "/etc/nginx/sites-enabled/jellyfin";

                string config = $@"server {{
  server_name {domainName};

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
    proxy_set_header Connection ""upgrade"";
  }}
}}";
                File.WriteAllText(availPath, config);
                symlink(availPath, enabledPath);

                if (RunCommand("sudo nginx -t") != 0)
                {
                    HandleErrorAndRollback(availPath, enabledPath);
                    return;
                }

                if (RunCommand($"sudo certbot --nginx -d {domainName}") != 0)
                {
                    HandleCertbotFailure(availPath, enabledPath);
                }

                RunCommand("sudo nginx -t");
                RunCommand("sudo systemctl reload nginx");
                Console.WriteLine($"SSL certificate sucessfully deployed! Visit \033[34mhttps://{domainName}\033[0m in your browser.\n\nExiting now...\n");
                return;
            }

            if (presets == "2")
            {
                Console.WriteLine("Make sure to follow my LEMP Stacks guide at \033[34mhttps://www.sizablesplash.com/server-guide/prereqs/OS-config/nextcloud/LEMP-stacks\033[0m\n");

                var psiPhp = new ProcessStartInfo
                {
                    FileName = "sh",
                    Arguments = "-c \"php -r 'echo PHP_MAJOR_VERSION.\\\".\\\".PHP_MINOR_VERSION;' 2>/dev/null\"",
                    RedirectStandardOutput = true,
                    UseShellExecute = false
                };
                using var pPhp = Process.Start(psiPhp);
                string phpVer = pPhp.StandardOutput.ReadToEnd().Trim();

                if (string.IsNullOrEmpty(phpVer))
                {
                    Console.WriteLine("\033[31mPHP not installed.\033[0m Exiting now...\n");
                    return;
                }

                string phpSock = $"/run/php/php{phpVer}-fpm.sock";
                Console.Write("Enter domain name: ");
                string domainName = Console.ReadLine()?.Trim();

                string availPath = "/etc/nginx/sites-available/nextcloud";
                string enabledPath = "/etc/nginx/sites-enabled/nextcloud";

                string config = $@"upstream php-handler {{
  server unix:{phpSock};
}}

map $arg_v $asset_immutable {{
  """" """";
  default "", immutable"";
}}

server {{
  listen 80;
  listen [::]:80;
  server_name {domainName};
  server_tokens off;
  return 301 https://$server_name$request_uri;
}}

server {{
  http2 on;
  server_name {domainName};
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

  add_header Referrer-Policy ""no-referrer"" always;
  add_header X-Content-Type-Options ""nosniff"" always;
  add_header X-Frame-Options ""SAMEORIGIN"" always;
  add_header X-Permitted-Cross-Domain-Policies ""none"" always;
  add_header X-Robots-Tag ""noindex, nofollow"" always;
  add_header Permissions-Policy ""camera=(), microphone=(), geolocation=()"" always;
  add_header Content-Security-Policy ""default-src 'self'; script-src 'self' 'unsafe-inline' 'unsafe-eval'; style-src 'self' 'unsafe-inline'; img-src 'self' data: blob:; font-src 'self'; connect-src 'self'; frame-ancestors 'self'"" always;

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
  location ~ ^/(?:\.|autotest|occ|issue|indie|db_|console) {{ return 404; }}

  location ~ ^/(?:composer\.(?:json|lock)|package(?:-lock)?\.json|core/shipped\.json)$ {{
    return 404;
  }}

  rewrite ^/(?!index|remote|public|cron|status|ocs\/v[12]|ocs-provider\/.+|core\/ajax\/update|updater\/.+|.+\/richdocumentscode(_arm64)?\/proxy) /index.php$request_uri;

  fastcgi_split_path_info ^(.+?\.php)(/.*)$;
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

  location ~ \.(?:css|js|mjs|svg|gif|ico|jpg|png|webp|wasm|tflite|map|ogg|flac|mp4|webm)$ {{
    try_files $uri /index.php$request_uri;
    add_header Cache-Control ""public, max-age=15778463$asset_immutable"" always;
    add_header Referrer-Policy ""no-referrer"" always;
    add_header X-Content-Type-Options ""nosniff"" always;
    add_header X-Frame-Options ""SAMEORIGIN"" always;
    add_header X-Permitted-Cross-Domain-Policies ""none"" always;
    add_header X-Robots-Tag ""noindex, nofollow"" always;
    access_log off;
  }}

  location ~ \.(otf|woff2?)$ {{
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
}}";
                File.WriteAllText(availPath, config);
                symlink(availPath, enabledPath);

                if (RunCommand("sudo nginx -t") != 0)
                {
                    HandleErrorAndRollback(availPath, enabledPath);
                    return;
                }

                if (RunCommand($"sudo certbot --nginx -d {domainName}") != 0)
                {
                    HandleCertbotFailure(availPath, enabledPath);
                }
                else
                {
                    Console.WriteLine($"SSL certificate sucessfully deployed! Visit \033[34mhttps://{domainName}\033[0m in your browser.\n\nExiting now...\n");
                    RunCommand("sudo nginx -t");
                    RunCommand("sudo systemctl reload nginx");
                }
                return;
            }

            if (presets == "3")
            {
                Console.Write("Enter domain name: ");
                string domainName = Console.ReadLine()?.Trim();
                string availPath = "/etc/nginx/sites-available/plex";
                string enabledPath = "/etc/nginx/sites-enabled/plex";

                string config = $@"server {{
  listen 80;
  server_name {domainName};

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
}}";
                File.WriteAllText(availPath, config);
                symlink(availPath, enabledPath);

                if (RunCommand("sudo nginx -t") != 0)
                {
                    HandleErrorAndRollback(availPath, enabledPath);
                    return;
                }

                if (RunCommand($"sudo certbot --nginx -d {domainName}") != 0)
                {
                    HandleCertbotFailure(availPath, enabledPath);
                }

                RunCommand("sudo nginx -t");
                RunCommand("sudo systemctl reload nginx");
                Console.WriteLine($"SSL certificate successfully deployed! Visit \033[0mhttps://{domainName}\033[0m in you browser.\n");
                return;
            }

            if (presets == "4")
            {
                Console.Write("\033[31mWARNING!!\033[0m Opening the auto config GUI to the public internet is risky. Do you wish to proceed? (y or n): ");
                string gui = Console.ReadLine()?.Trim();
                if (gui == "n") return;

                Console.Write("Enter domain name: ");
                string domainName = Console.ReadLine()?.Trim();

                RunCommand("rm -f /etc/nginx/sites-available/auto-config /etc/nginx/sites-enabled/auto-config");

                string availPath = "/etc/nginx/sites-available/auto-config";
                string enabledPath = "/etc/nginx/sites-enabled/auto-config";

                string config = $@"server {{
  listen 80;
  listen [::]:80;

  server_name {domainName};

  root /var/www/auto-config/;
  index index.html index.php;

  location / {{
    autoindex on;
    try_files $uri $uri/ $uri/index.html $uri.html $uri.php =404;
  }}
}}";
                File.WriteAllText(availPath, config);
                symlink(availPath, enabledPath);

                if (RunCommand("sudo nginx -t") != 0)
                {
                    HandleErrorAndRollback(availPath, enabledPath);
                    return;
                }

                if (RunCommand($"sudo certbot --nginx -d {domainName}") != 0)
                {
                    HandleCertbotFailure(availPath, enabledPath);
                }
                else
                {
                    Console.WriteLine($"SSL certificate sucessfully deployed! Visit \033[34mhttps://{domainName}\033[0m in your browser.\n\nExiting now...\n");
                    RunCommand("sudo nginx -t");
                    RunCommand("sudo systemctl reload nginx");
                }
                return;
            }
        }

        if (preset == "q") return;

        Console.Write("Input 1 for reverse proxy or 2 for standalone directory: ");
        string type = Console.ReadLine()?.Trim();

        string proxy = "", directory = "";
        if (type == "1")
        {
            Console.Write("Enter the proxy port: ");
            proxy = Console.ReadLine()?.Trim();
        }
        else if (type == "2")
        {
            Console.Write("Enter directory path: ");
            directory = Console.ReadLine()?.Trim();
        }

        Console.Write("Enter name of the config: ");
        string configName = Console.ReadLine()?.Trim();

        Console.Write("Enter domain name: ");
        string domainName = Console.ReadLine()?.Trim();

        string availPath = $"/etc/nginx/sites-available/{configName}";
        string enabledPath = $"/etc/nginx/sites-enabled/{configName}";

        string config = "";
        if (type == "1")
        {
            config = $@"server {{
  server_name {domainName};

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
    proxy_set_header Connection ""upgrade"";
  }}
}}";
        }
        else if (type == "2")
        {
            bool hasPhp = RunCommand("php -v 1>/dev/null 2>&1") == 0;
            if (hasPhp)
            {
                config = $@"server {{
  listen 80;
  listen [::]:80;

  server_name {domainName};

  root {directory};
  index index.html index.htm index.php;

  location / {{
    autoindex on;
    try_files $uri $uri/ $uri/index.html $uri.html $uri.php =404;
  }}
}}";
            }
            else
            {
                config = $@"server {{
  listen 80;
  listen [::]:80;

  server_name {domainName};

  root {directory};
  index index.html index.htm;

  location / {{
    autoindex on;
    try_files $uri $uri/ $uri/index.html $uri.html =404;
  }}
}}";
            }
        }

        File.WriteAllText(availPath, config);
        symlink(availPath, enabledPath);

        if (RunCommand("sudo nginx -t") != 0)
        {
            HandleErrorAndRollback(availPath, enabledPath);
            return;
        }

        if (RunCommand($"sudo certbot --nginx -d {domainName}") != 0)
        {
            HandleCertbotFailure(availPath, enabledPath);
        }
        else
        {
            Console.WriteLine($"SSL certificate sucessfully deployed! Visit \033[34mhttps://{domainName}\033[0m in your browser.\n\nExiting now...\n");
            RunCommand("sudo nginx -t");
            RunCommand("sudo systemctl reload nginx");
        }
    }
}
