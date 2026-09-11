#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "/mnt/code-projects/packages/auto-config/version 3.0/config.h"

void run_nextcloud(void)
{
  FILE *cmd = popen("php -r 'echo PHP_MAJOR_VERSION.\".\".PHP_MINOR_VERSION;' 2>/dev/null", "r");

  if (cmd == NULL || fgets(php_ver, sizeof(php_ver), cmd) == NULL || strlen(php_ver) == 0)
  {
    if (cmd != NULL)
    {
      pclose(cmd);
    }
  
    printf("\033[31mPHP not installed.\033[0m Exiting now...\n\n");
    return;
  }

  pclose(cmd);

  php_ver[strcspn(php_ver, "\r\n")] = 0;

  run_domain_name();

  snprintf(avail_path, sizeof(avail_path), "/etc/nginx/sites-available/nextcloud");
  FILE *fp = fopen(avail_path, "w");
  fprintf(fp,
"upstream php-handler {\n"
        "  server unix:/run/php/php%s-fpm.sock;\n"
        "}\n\n"
        "map $arg_v $asset_immutable {\n"
        "  \"\" \"\";\n"
        "  default \", immutable\";\n"
        "}\n\n"
        "server {\n"
        "  listen 80;\n"
        "  listen [::]:80;\n"
        "  server_name %s;\n"
        "  server_tokens off;\n"
        "  return 301 https://$server_name$request_uri;\n"
        "}\n\n"
        "server {\n"
        "  http2 on;\n"
        "  server_name %s;\n"
        "  root /var/www/nextcloud;\n"
        "  server_tokens off;\n\n"
        "  client_max_body_size 512M;\n"
        "  client_body_timeout 300s;\n"
        "  fastcgi_buffers 64 4K;\n\n"
        "  gzip on;\n"
        "  gzip_vary on;\n"
        "  gzip_comp_level 4;\n"
        "  gzip_min_length 256;\n"
        "  gzip_proxied expired no-cache no-store private no_last_modified no_etag auth;\n"
        "  gzip_types application/atom+xml text/javascript application/javascript application/json application/ld+json application/manifest+json application/rss+xml application/vnd.geo+json application/vnd.ms-fontobject application/wasm application/x-font-ttf application/x-web-app-manifest+json application/xhtml+xml application/xml font/opentype image/bmp image/svg+xml image/x-icon text/cache-manifest text/css text/plain text/vcard text/vnd.rim.location.xloc text/vtt text/x-component text/x-cross-domain-policy;\n\n"
        "  client_body_buffer_size 512k;\n\n"
        "  add_header Referrer-Policy \"no-referrer\" always;\n"
        "  add_header X-Content-Type-Options \"nosniff\" always;\n"
        "  add_header X-Frame-Options \"SAMEORIGIN\" always;\n"
        "  add_header X-Permitted-Cross-Domain-Policies \"none\" always;\n"
        "  add_header X-Robots-Tag \"noindex, nofollow\" always;\n"
        "  add_header Permissions-Policy \"camera=(), microphone=(), geolocation=()\" always;\n"
        "  add_header Content-Security-Policy \"default-src 'self'; script-src 'self' 'unsafe-inline' 'unsafe-eval'; style-src 'self' 'unsafe-inline'; img-src 'self' data: blob:; font-src 'self'; connect-src 'self'; frame-ancestors 'self'\" always;\n\n"
        "  fastcgi_hide_header X-Powered-By;\n\n"
        "  include mime.types;\n"
        "  types {\n"
        "    text/javascript mjs;\n"
        "    application/wasm wasm;\n"
        "  }\n\n"
        "  index index.php index.html /index.php$request_uri;\n\n"
        "  location = / {\n"
        "    if ( $http_user_agent ~ ^DavClnt ) {\n"
        "      return 302 /remote.php/webdav/$is_args$args;\n"
        "    }\n"
        "  }\n\n"
        "  location = /robots.txt {\n"
        "    allow all;\n"
        "    log_not_found off;\n"
        "    access_log off;\n"
        "  }\n\n"
        "  location ^~ /.well-known {\n"
        "    location = /.well-known/carddav { return 301 /remote.php/dav/; }\n"
        "    location = /.well-known/caldav { return 301 /remote.php/dav/; }\n"
        "    location /.well-known/acme-challenge { try_files $uri $uri/ =404; }\n"
        "    location /.well-known/pki-validation { try_files $uri $uri/ =404; }\n"
        "    return 301 /index.php$request_uri;\n"
        "  }\n\n"
        "  location ~ ^/(?:build|tests|config|lib|3rdparty|templates|data)(?:$|/) { return 404; }\n"
        "  location ~ ^/(?:\\.|autotest|occ|issue|indie|db_|console) { return 404; }\n\n"
        "  location ~ ^/(?:composer\\.(?:json|lock)|package(?:-lock)?\\.json|core/shipped\\.json)$ {\n"
        "    return 404;\n"
        "  }\n\n"
        "  rewrite ^/(?!index|remote|public|cron|status|ocs\\/v[12]|ocs-provider\\/.+|core\\/ajax\\/update|updater\\/.+|.+\\/richdocumentscode(_arm64)?\\/proxy) /index.php$request_uri;\n\n"
        "  fastcgi_split_path_info ^(.+?\\.php)(/.*)$;\n"
        "  set $path_info $fastcgi_path_info;\n\n"
        "  try_files $fastcgi_script_name =404;\n\n"
        "  include fastcgi_params;\n"
        "  fastcgi_pass php-handler;\n\n"
        "  fastcgi_param SCRIPT_FILENAME $document_root$fastcgi_script_name;\n"
        "  fastcgi_param PATH_INFO $path_info;\n"
        "  fastcgi_param HTTPS on;\n"
        "  fastcgi_param modHeadersAvailable true;\n"
        "  fastcgi_param front_controller_active true;\n"
        "  fastcgi_max_temp_file_size 0;\n\n"
        "  location ~ \\.(?:css|js|mjs|svg|gif|ico|jpg|png|webp|wasm|tflite|map|ogg|flac|mp4|webm)$ {\n"
        "    try_files $uri /index.php$request_uri;\n"
        "    add_header Cache-Control \"public, max-age=15778463$asset_immutable\" always;\n"
        "    add_header Referrer-Policy \"no-referrer\" always;\n"
        "    add_header X-Content-Type-Options \"nosniff\" always;\n"
        "    add_header X-Frame-Options \"SAMEORIGIN\" always;\n"
        "    add_header X-Permitted-Cross-Domain-Policies \"none\" always;\n"
        "    add_header X-Robots-Tag \"noindex, nofollow\" always;\n"
        "    access_log off;\n"
        "  }\n\n"
        "  location ~ \\.(otf|woff2?)$ {\n"
        "    try_files $uri /index.php$request_uri;\n"
        "    expires 7d;\n"
        "    access_log off;\n"
        "  }\n\n"
        "  location /remote {\n"
        "    return 301 /remote.php$request_uri;\n"
        "  }\n\n"
        "  location / {\n"
        "    try_files $uri $uri/ /index.php$request_uri;\n"
        "  }\n"
        "}\n",
        php_ver,
        domain_name,
        domain_name
  );

  snprintf(enabled_path, sizeof(enabled_path), "/etc/nginx/sites-enabled/nextcloud");
  symlink(avail_path, enabled_path);

  certbot();

  return;
}