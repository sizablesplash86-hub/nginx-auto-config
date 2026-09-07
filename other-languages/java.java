import java.io.*;
import java.net.*;
import java.nio.file.*;
import java.util.Scanner;

public class NginxAutoConfig {
    private static final String CURRENT_VERSION = "v2.0.0";
    private static final String REPO_URL = "https://api.github.com/repos/sizablesplash86-hub/nginx-auto-config/releases/latest";

    private static int rootCheck() {
        try {
            Process process = Runtime.getRuntime().exec(new String[]{"id", "-u"});
            BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
            String line = reader.readLine();
            if (line != null && !line.trim().equals("0")) {
                System.out.println("\n\033[31mEnter root first\033[0m\n");
                return 1;
            }
        } catch (Exception e) {
            return 1;
        }
        return 0;
    }

    private static void updateCheck(Scanner scanner) {
        try {
            Process process = Runtime.getRuntime().exec(new String[]{
                "sh", "-c", "curl -s -H \"User-Agent: auto-config-app\" " + REPO_URL + " | grep '\"tag_name\":' | sed -E 's/.*\"([^\"]+)\".*/\\1/'"
            });
            BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
            String latestVersion = reader.readLine();

            if (latestVersion != null && !latestVersion.trim().isEmpty() && !latestVersion.trim().equals(CURRENT_VERSION)) {
                latestVersion = latestVersion.trim();
                System.out.print(CURRENT_VERSION + " --> " + latestVersion);
                System.out.print("Would you like to upgrade? (y or n): ");
                String ans = scanner.nextLine().trim();

                if (ans.equals("y")) {
                    System.out.println("\nUpgrading to " + latestVersion + "...\n");
                    String verNum = latestVersion.startsWith("v") ? latestVersion.substring(1) : latestVersion;
                    String updateCmd = String.format(
                        "curl -sfL https://github.com/sizablesplash86-hub/nginx-auto-config/releases/download/%s/auto-config_%s_amd64.deb -o /tmp/auto-config_update.deb && " +
                        "dpkg -i /tmp/auto-config_update.deb || apt-get install -f -y && " +
                        "rm -f /tmp/auto-config_update.deb", latestVersion, verNum
                    );

                    Process pUpdate = Runtime.getRuntime().exec(new String[]{"sh", "-c", updateCmd});
                    if (pUpdate.waitFor() == 0) {
                        System.out.println("\nUpgrade to " + latestVersion + " completed successfully!\n");
                        Runtime.getRuntime().exec("auto-config");
                        System.exit(0);
                    } else {
                        System.out.println("\nUPGRADE FALIED! Continuing with current version...\n");
                    }
                }
            }
        } catch (Exception ignored) {}
    }

    private static String getLanIp() {
        try (DatagramSocket socket = new DatagramSocket()) {
            socket.connect(InetAddress.getByName("8.8.8.8"), 10002);
            return socket.getLocalAddress().getHostAddress();
        } catch (Exception e) {
            return "127.0.0.1";
        }
    }

    private static void handleErrorAndRollback(String availPath, String enabledPath) throws Exception {
        System.out.println("\033[31mUNKNOWN ERROR OCCURRED\033[0m");
        System.out.println("Removing broken configuration...");
        Files.deleteIfExists(Paths.get(availPath));
        Files.deleteIfExists(Paths.get(enabledPath));
        Runtime.getRuntime().exec(new String[]{"sudo", "nginx", "-t"}).waitFor();
    }

    private static void handleCertbotFailure(Scanner scanner, String availPath, String enabledPath) throws Exception {
        System.out.println("\033[31mERROR\033[0m SSL certificate failed to deploy\n");
        System.out.print("Would you like to remove config in /etc/nginx/sites-available /etc/nginx/sites-enabled? (y or n): ");
        String rem = scanner.nextLine().trim();
        if (rem.equals("y")) {
            System.out.println("Removing broken configuration...");
            Files.deleteIfExists(Paths.get(availPath));
            Files.deleteIfExists(Paths.get(enabledPath));
            Runtime.getRuntime().exec(new String[]{"sudo", "nginx", "-t"}).waitFor();
            System.exit(0);
        } else if (rem.equals("n") || rem.equals("q")) {
            System.out.println("Exiting now...\n");
            System.exit(0);
        }
    }

    public static void main(String[] args) throws Exception {
        if (rootCheck() != 0) return;

        Scanner scanner = new Scanner(System.in);
        updateCheck(scanner);

        String lanIp = getLanIp();

        System.out.printf("\nWelcome to the NGINX Auto Config %s! Now known as N.A.P. for NGINX Auto Program because it's as easy as taking a NAP.\n\n", CURRENT_VERSION);
        System.out.printf("Visit \033[34mhttp://%s:3487\033[0m in the browser to use the graphical interface\n\n", lanIp);
        System.out.println("This program is designed to work with my server guide \033[34mhttps://www.sizablesplash.com/server-guide\033[0m\n");
        System.out.println("at any prompt, input q to exit\n");

        System.out.print("Would you like to pick between presets? (y or n): ");
        String preset = scanner.nextLine().trim();

        if (preset.equals("y")) {
            System.out.print("Input 1 for Jellyfin, 2 for Nextcloud LEMP stacks, 3 for Plex, 4 for auto config GUI (NOT RECOMMENDED): ");
            String presets = scanner.nextLine().trim();

            if (presets.equals("1")) {
                System.out.print("Enter domain name: ");
                String domainName = scanner.nextLine().trim();
                String availPath = "/etc/nginx/sites-available/jellyfin";
                String enabledPath = "/etc/nginx/sites-enabled/jellyfin";

                String config = String.format(
                    "server {\n" +
                    "  server_name %s;\n\n" +
                    "  location / {\n" +
                    "    proxy_pass http://127.0.0.1:8096;\n" +
                    "    proxy_set_header Host $host;\n" +
                    "    proxy_set_header X-Real-IP $remote_addr;\n" +
                    "    proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;\n" +
                    "    proxy_set_header X-Forwarded-Proto $scheme;\n" +
                    "    proxy_set_header X-Forwarded-Protocol $scheme;\n" +
                    "    proxy_set_header X-Forwarded-Host $http_host;\n" +
                    "    proxy_http_version 1.1;\n" +
                    "    proxy_set_header Upgrade $http_upgrade;\n" +
                    "    proxy_set_header Connection \"upgrade\";\n" +
                    "  }\n" +
                    "}\n", domainName
                );

                Files.writeString(Paths.get(availPath), config);
                try { Files.createSymbolicLink(Paths.get(enabledPath), Paths.get(availPath)); } catch (Exception ignored) {}

                if (Runtime.getRuntime().exec(new String[]{"sudo", "nginx", "-t"}).waitFor() != 0) {
                    handleErrorAndRollback(availPath, enabledPath);
                    return;
                }

                if (Runtime.getRuntime().exec(new String[]{"sudo", "certbot", "--nginx", "-d", domainName}).waitFor() != 0) {
                    handleCertbotFailure(scanner, availPath, enabledPath);
                }

                Runtime.getRuntime().exec(new String[]{"sudo", "nginx", "-t"}).waitFor();
                Runtime.getRuntime().exec(new String[]{"sudo", "systemctl", "reload", "nginx"}).waitFor();
                System.out.printf("SSL certificate sucessfully deployed! Visit \033[34mhttps://%s\033[0m in your browser.\n\nExiting now...\n\n", domainName);
                return;
            }

            if (presets.equals("2")) {
                System.out.println("Make sure to follow my LEMP Stacks guide at \033[34mhttps://www.sizablesplash.com/server-guide/prereqs/OS-config/nextcloud/LEMP-stacks\033[0m\n");

                Process pPhp = Runtime.getRuntime().exec(new String[]{"sh", "-c", "php -r 'echo PHP_MAJOR_VERSION.\".\".PHP_MINOR_VERSION;' 2>/dev/null"});
                BufferedReader rPhp = new BufferedReader(new InputStreamReader(pPhp.getInputStream()));
                String phpVer = rPhp.readLine();

                if (phpVer == null || phpVer.trim().isEmpty()) {
                    System.out.println("\033[31mPHP not installed.\033[0m Exiting now...\n");
                    return;
                }

                String phpSock = String.format("/run/php/php%s-fpm.sock", phpVer.trim());
                System.out.print("Enter domain name: ");
                String domainName = scanner.nextLine().trim();

                String availPath = "/etc/nginx/sites-available/nextcloud";
                String enabledPath = "/etc/nginx/sites-enabled/nextcloud";

                String config = String.format(
                    "upstream php-handler {\n" +
                    "  server unix:%s;\n" +
                    "}\n\n" +
                    "map $arg_v $asset_immutable {\n" +
                    "  \"\" \"\";\n" +
                    "  default \", immutable\";\n" +
                    "}\n\n" +
                    "server {\n" +
                    "  listen 80;\n" +
                    "  listen [::]:80;\n" +
                    "  server_name %s;\n" +
                    "  server_tokens off;\n" +
                    "  return 301 https://$server_name$request_uri;\n" +
                    "}\n\n" +
                    "server {\n" +
                    "  http2 on;\n" +
                    "  server_name %s;\n" +
                    "  root /var/www/nextcloud;\n" +
                    "  server_tokens off;\n\n" +
                    "  client_max_body_size 512M;\n" +
                    "  client_body_timeout 300s;\n" +
                    "  fastcgi_buffers 64 4K;\n\n" +
                    "  gzip on;\n" +
                    "  gzip_vary on;\n" +
                    "  gzip_comp_level 4;\n" +
                    "  gzip_min_length 256;\n" +
                    "  gzip_proxied expired no-cache no-store private no_last_modified no_etag auth;\n" +
                    "  gzip_types application/atom+xml text/javascript application/javascript application/json application/ld+json application/manifest+json application/rss+xml application/vnd.geo+json application/vnd.ms-fontobject application/wasm application/x-font-ttf application/x-web-app-manifest+json application/xhtml+xml application/xml font/opentype image/bmp image/svg+xml image/x-icon text/cache-manifest text/css text/plain text/vcard text/vnd.rim.location.xloc text/vtt text/x-component text/x-cross-domain-policy;\n\n" +
                    "  client_body_buffer_size 512k;\n\n" +
                    "  add_header Referrer-Policy \"no-referrer\" always;\n" +
                    "  add_header X-Content-Type-Options \"nosniff\" always;\n" +
                    "  add_header X-Frame-Options \"SAMEORIGIN\" always;\n" +
                    "  add_header X-Permitted-Cross-Domain-Policies \"none\" always;\n" +
                    "  add_header X-Robots-Tag \"noindex, nofollow\" always;\n" +
                    "  add_header Permissions-Policy \"camera=(), microphone=(), geolocation=()\" always;\n" +
                    "  add_header Content-Security-Policy \"default-src 'self'; script-src 'self' 'unsafe-inline' 'unsafe-eval'; style-src 'self' 'unsafe-inline'; img-src 'self' data: blob:; font-src 'self'; connect-src 'self'; frame-ancestors 'self'\" always;\n\n" +
                    "  fastcgi_hide_header X-Powered-By;\n\n" +
                    "  include mime.types;\n" +
                    "  types {\n" +
                    "    text/javascript mjs;\n" +
                    "    application/wasm wasm;\n" +
                    "  }\n\n" +
                    "  index index.php index.html /index.php$request_uri;\n\n" +
                    "  location = / {\n" +
                    "    if ( $http_user_agent ~ ^DavClnt ) {\n" +
                    "      return 302 /remote.php/webdav/$is_args$args;\n" +
                    "    }\n" +
                    "  }\n\n" +
                    "  location = /robots.txt {\n" +
                    "    allow all;\n" +
                    "    log_not_found off;\n" +
                    "    access_log off;\n" +
                    "  }\n\n" +
                    "  location ^~ /.well-known {\n" +
                    "    location = /.well-known/carddav { return 301 /remote.php/dav/; }\n" +
                    "    location = /.well-known/caldav { return 301 /remote.php/dav/; }\n" +
                    "    location /.well-known/acme-challenge { try_files $uri $uri/ =404; }\n" +
                    "    location /.well-known/pki-validation { try_files $uri $uri/ =404; }\n" +
                    "    return 301 /index.php$request_uri;\n" +
                    "  }\n\n" +
                    "  location ~ ^/(?:build|tests|config|lib|3rdparty|templates|data)(?:$|/) { return 404; }\n" +
                    "  location ~ ^/(?:\\.|autotest|occ|issue|indie|db_|console) { return 404; }\n\n" +
                    "  location ~ ^/(?:composer\\.(?:json|lock)|package(?:-lock)?\\.json|core/shipped\\.json)$ {\n" +
                    "    return 404;\n" +
                    "  }\n\n" +
                    "  rewrite ^/(?!index|remote|public|cron|status|ocs\\/v[12]|ocs-provider\\/.+|core\\/ajax\\/update|updater\\/.+|.+\\/richdocumentscode(_arm64)?\\/proxy) /index.php$request_uri;\n\n" +
                    "  fastcgi_split_path_info ^(.+?\\.php)(/.*)$;\n" +
                    "  set $path_info $fastcgi_path_info;\n\n" +
                    "  try_files $fastcgi_script_name =404;\n\n" +
                    "  include fastcgi_params;\n" +
                    "  fastcgi_pass php-handler;\n\n" +
                    "  fastcgi_param SCRIPT_FILENAME $document_root$fastcgi_script_name;\n" +
                    "  fastcgi_param PATH_INFO $path_info;\n" +
                    "  fastcgi_param HTTPS on;\n" +
                    "  fastcgi_param modHeadersAvailable true;\n" +
                    "  fastcgi_param front_controller_active true;\n" +
                    "  fastcgi_max_temp_file_size 0;\n\n" +
                    "  location ~ \\.(?:css|js|mjs|svg|gif|ico|jpg|png|webp|wasm|tflite|map|ogg|flac|mp4|webm)$ {\n" +
                    "    try_files $uri /index.php$request_uri;\n" +
                    "    add_header Cache-Control \"public, max-age=15778463$asset_immutable\" always;\n" +
                    "    add_header Referrer-Policy \"no-referrer\" always;\n" +
                    "    add_header X-Content-Type-Options \"nosniff\" always;\n" +
                    "    add_header X-Frame-Options \"SAMEORIGIN\" always;\n" +
                    "    add_header X-Permitted-Cross-Domain-Policies \"none\" always;\n" +
                    "    add_header X-Robots-Tag \"noindex, nofollow\" always;\n" +
                    "    access_log off;\n" +
                    "  }\n\n" +
                    "  location ~ \\.(otf|woff2?)$ {\n" +
                    "    try_files $uri /index.php$request_uri;\n" +
                    "    expires 7d;\n" +
                    "    access_log off;\n" +
                    "  }\n\n" +
                    "  location /remote {\n" +
                    "    return 301 /remote.php$request_uri;\n" +
                    "  }\n\n" +
                    "  location / {\n" +
                    "    try_files $uri $uri/ /index.php$request_uri;\n" +
                    "  }\n" +
                    "}\n", phpSock, domainName, domainName
                );

                Files.writeString(Paths.get(availPath), config);
                try { Files.createSymbolicLink(Paths.get(enabledPath), Paths.get(availPath)); } catch (Exception ignored) {}

                if (Runtime.getRuntime().exec(new String[]{"sudo", "nginx", "-t"}).waitFor() != 0) {
                    handleErrorAndRollback(availPath, enabledPath);
                    return;
                }

                if (Runtime.getRuntime().exec(new String[]{"sudo", "certbot", "--nginx", "-d", domainName}).waitFor() != 0) {
                    handleCertbotFailure(scanner, availPath, enabledPath);
                } else {
                    System.out.printf("SSL certificate sucessfully deployed! Visit \033[34mhttps://%s\033[0m in your browser.\n\nExiting now...\n\n", domainName);
                    Runtime.getRuntime().exec(new String[]{"sudo", "nginx", "-t"}).waitFor();
                    Runtime.getRuntime().exec(new String[]{"sudo", "systemctl", "reload", "nginx"}).waitFor();
                }
                return;
            }

            if (presets.equals("3")) {
                System.out.print("Enter domain name: ");
                String domainName = scanner.nextLine().trim();
                String availPath = "/etc/nginx/sites-available/plex";
                String enabledPath = "/etc/nginx/sites-enabled/plex";

                String config = String.format(
                    "server {\n" +
                    "  listen 80;\n" +
                    "  server_name %s;\n\n" +
                    "  location / {\n" +
                    "    proxy_pass http://127.0.0.1:3400;\n" +
                    "    proxy_http_version 1.1;\n" +
                    "    proxy_set_header Upgrade $http_upgrade;\n" +
                    "    proxy_set_header Connection 'upgrade';\n" +
                    "    proxy_set_header Host $host;\n" +
                    "    proxy_cache_bypass $http_upgrade;\n" +
                    "    proxy_set_header X-Real-IP $remote_addr;\n" +
                    "  }\n\n" +
                    "  error_page 502 /502.html;\n" +
                    "  location = /502.html {\n" +
                    "    root /home/PlexStore;\n" +
                    "  }\n" +
                    "}\n", domainName
                );

                Files.writeString(Paths.get(availPath), config);
                try { Files.createSymbolicLink(Paths.get(enabledPath), Paths.get(availPath)); } catch (Exception ignored) {}

                if (Runtime.getRuntime().exec(new String[]{"sudo", "nginx", "-t"}).waitFor() != 0) {
                    handleErrorAndRollback(availPath, enabledPath);
                    return;
                }

                if (Runtime.getRuntime().exec(new String[]{"sudo", "certbot", "--nginx", "-d", domainName}).waitFor() != 0) {
                    handleCertbotFailure(scanner, availPath, enabledPath);
                }

                Runtime.getRuntime().exec(new String[]{"sudo", "nginx", "-t"}).waitFor();
                Runtime.getRuntime().exec(new String[]{"sudo", "systemctl", "reload", "nginx"}).waitFor();
                System.out.printf("SSL certificate successfully deployed! Visit \033[0mhttps://%s\033[0m in you browser.\n\n", domainName);
                return;
            }

            if (presets.equals("4")) {
                System.out.print("\033[31mWARNING!!\033[0m Opening the auto config GUI to the public internet is risky. Do you wish to proceed? (y or n): ");
                String gui = scanner.nextLine().trim();
                if (gui.equals("n")) return;

                System.out.print("Enter domain name: ");
                String domainName = scanner.nextLine().trim();

                Runtime.getRuntime().exec(new String[]{"sh", "-c", "rm -f /etc/nginx/sites-available/auto-config /etc/nginx/sites-enabled/auto-config"}).waitFor();

                String availPath = "/etc/nginx/sites-available/auto-config";
                String enabledPath = "/etc/nginx/sites-enabled/auto-config";

                String config = String.format(
                    "server {\n" +
                    "  listen 80;\n" +
                    "  listen [::]:80;\n\n" +
                    "  server_name %s;\n\n" +
                    "  root /var/www/auto-config/;\n" +
                    "  index index.html index.php;\n\n" +
                    "  location / {\n" +
                    "    autoindex on;\n" +
                    "    try_files $uri $uri/ $uri/index.html $uri.html $uri.php =404;\n" +
                    "  }\n" +
                    "}\n", domainName
                );

                Files.writeString(Paths.get(availPath), config);
                try { Files.createSymbolicLink(Paths.get(enabledPath), Paths.get(availPath)); } catch (Exception ignored) {}

                if (Runtime.getRuntime().exec(new String[]{"sudo", "nginx", "-t"}).waitFor() != 0) {
                    handleErrorAndRollback(availPath, enabledPath);
                    return;
                }

                if (Runtime.getRuntime().exec(new String[]{"sudo", "certbot", "--nginx", "-d", domainName}).waitFor() != 0) {
                    handleCertbotFailure(scanner, availPath, enabledPath);
                } else {
                    System.out.printf("SSL certificate sucessfully deployed! Visit \033[34mhttps://%s\033[0m in your browser.\n\nExiting now...\n\n", domainName);
                    Runtime.getRuntime().exec(new String[]{"sudo", "nginx", "-t"}).waitFor();
                    Runtime.getRuntime().exec(new String[]{"sudo", "systemctl", "reload", "nginx"}).waitFor();
                }
                return;
            }
        }

        if (preset.equals("q")) return;

        System.out.print("Input 1 for reverse proxy or 2 for standalone directory: ");
        String type = scanner.nextLine().trim();

        String proxy = "", directory = "";
        if (type.equals("1")) {
            System.out.print("Enter the proxy port: ");
            proxy = scanner.nextLine().trim();
        } else if (type.equals("2")) {
            System.out.print("Enter directory path: ");
            directory = scanner.nextLine().trim();
        }

        System.out.print("Enter name of the config: ");
        String configName = scanner.nextLine().trim();

        System.out.print("Enter domain name: ");
        String domainName = scanner.nextLine().trim();

        String availPath = "/etc/nginx/sites-available/" + configName;
        String enabledPath = "/etc/nginx/sites-enabled/" + configName;

        String config = "";
        if (type.equals("1")) {
            config = String.format(
                "server {\n" +
                "  server_name %s;\n\n" +
                "  location / {\n" +
                "    proxy_pass http://127.0.0.1:%s;\n" +
                "    proxy_set_header Host $host;\n" +
                "    proxy_set_header X-Real-IP $remote_addr;\n" +
                "    proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;\n" +
                "    proxy_set_header X-Forwarded-Proto $scheme;\n" +
                "    proxy_set_header X-Forwarded-Protocol $scheme;\n" +
                "    proxy_set_header X-Forwarded-Host $http_host;\n" +
                "    proxy_http_version 1.1;\n" +
                "    proxy_set_header Upgrade $http_upgrade;\n" +
                "    proxy_set_header Connection \"upgrade\";\n" +
                "  }\n" +
                "}\n", domainName, proxy
            );
        } else if (type.equals("2")) {
            boolean hasPhp = Runtime.getRuntime().exec(new String[]{"sh", "-c", "php -v 1>/dev/null 2>&1"}).waitFor() == 0;
            if (hasPhp) {
                config = String.format(
                    "server {\n" +
                    "  listen 80;\n" +
                    "  listen [::]:80;\n\n" +
                    "  server_name %s;\n\n" +
                    "  root %s;\n" +
                    "  index index.html index.htm index.php;\n\n" +
                    "  location / {\n" +
                    "    autoindex on;\n" +
                    "    try_files $uri $uri/ $uri/index.html $uri.html $uri.php =404;\n" +
                    "  }\n" +
                    "}\n", domainName, directory
                );
            } else {
                config = String.format(
                    "server {\n" +
                    "  listen 80;\n" +
                    "  listen [::]:80;\n\n" +
                    "  server_name %s;\n\n" +
                    "  root %s;\n" +
                    "  index index.html index.htm;\n\n" +
                    "  location / {\n" +
                    "    autoindex on;\n" +
                    "    try_files $uri $uri/ $uri/index.html $uri.html =404;\n" +
                    "  }\n" +
                    "}\n", domainName, directory
                );
            }
        }

        Files.writeString(Paths.get(availPath), config);
        try { Files.createSymbolicLink(Paths.get(enabledPath), Paths.get(availPath)); } catch (Exception ignored) {}

        if (Runtime.getRuntime().exec(new String[]{"sudo", "nginx", "-t"}).waitFor() != 0) {
            handleErrorAndRollback(availPath, enabledPath);
            return;
        }

        if (Runtime.getRuntime().exec(new String[]{"sudo", "certbot", "--nginx", "-d", domainName}).waitFor() != 0) {
            handleCertbotFailure(scanner, availPath, enabledPath);
        } else {
            System.out.printf("SSL certificate sucessfully deployed! Visit \033[34mhttps://%s\033[0m in your browser.\n\nExiting now...\n\n", domainName);
            Runtime.getRuntime().exec(new String[]{"sudo", "nginx", "-t"}).waitFor();
            Runtime.getRuntime().exec(new String[]{"sudo", "systemctl", "reload", "nginx"}).waitFor();
        }
    }
}
