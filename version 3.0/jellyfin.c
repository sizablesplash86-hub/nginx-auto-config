#include <stdio.h>
#include <unistd.h>
#include "config.h"

void run_jellyfin(void)
{
  run_domain_name();

  snprintf(avail_path, sizeof(avail_path), "/etc/nginx/sites-available/jellyfin");
  FILE *fp = fopen(avail_path, "w");

  fprintf(fp,
    "server {\n"
    "  listen 443 ssl;\n"
    "  listen [::]:443 ssl;\n"
    "  http2 on;\n\n"
    "  server_name %s;\n\n"
    "  client_max_body_size 20M;\n\n"
    "  ssl_protocols TLSv1.3 TLSv1.2;\n\n"
    "#  include /etc/letsencrypt/options-ssl-nginx.conf;\n"
    "  ssl_dhparam /etc/letsencrypt/ssl-dhparams.pem;\n\n"
    "  set $jellyfin 127.0.0.1;\n"
    "  add_header X-Content-Type-Options \"nosniff\";\n"
    "  add_header Permissions-Policy \"accelerometer=(), ambient-light-sensor=(), battery=(), bluetooth=(), camera=(), clipboard-read=(), display-capture=(), document-domain=(), encrypted-media=(), gamepad=(), geolocation=(), gyroscope=(), hid=(), idle-detection=(), interest-cohort=(), keyboard-map=(), local-fonts=(), magnetometer=(), microphone=(), payment=(), publickey-credentials-get=(), serial=(), sync-xhr=(), usb=(), xr-spatial-tracking=()\" always;\n"
    "  add_header Content-Security-Policy \"default-src https: data: blob: ; img-src 'self' https://* ; style-src 'self' 'unsafe-inline'; script-src 'self' 'unsafe-inline' https://www.gstatic.com https://www.youtube.com blob:; worker-src 'self' blob:; connect-src 'self'; object-src 'none'; font-src 'self'\";\n\n"
    "  location / {\n"
    "    proxy_pass http://$jellyfin:8096;\n"
    "    proxy_set_header Host $host;\n"
    "    proxy_set_header X-Real-IP $remote_addr;\n"
    "    proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;\n"
    "    proxy_set_header X-Forwarded-Proto $scheme;\n"
    "    proxy_set_header X-Forwarded-Protocol $scheme;\n"
    "    proxy_set_header X-Forwarded-Host $http_host;\n"
    "    proxy_buffering off;\n"
    "  }\n\n"
    "  location /socket {\n"
    "    proxy_pass http://$jellyfin:8096;\n"
    "    proxy_http_version 1.1;\n"
    "    proxy_set_header Upgrade $http_upgrade;\n"
    "    proxy_set_header Connection \"upgrade\";\n"
    "    proxy_set_header Host $host;\n"
    "    proxy_set_header X-Real-IP $remote_addr;\n"
    "    proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;\n"
    "    proxy_set_header X-Forwarded-Proto $scheme;\n"
    "    proxy_set_header X-Forwarded-Protocol $scheme;\n"
    "    proxy_set_header X-Forwarded-Host $http_host;\n"
    "  }\n"
    "}\n\n"
    "server {\n"
    "  listen 80;\n"
    "  listen [::]:80;\n"
    "  server_name %s;\n"
    "  return 301 https://$host$request_uri;\n"
    "}\n",
    domain_name, domain_name
  );

  fclose(fp);

  snprintf(enabled_path, sizeof(enabled_path), "/etc/nginx/sites-enabled/jellyfin");
  symlink(avail_path, enabled_path);

  certbot();

  return;
}