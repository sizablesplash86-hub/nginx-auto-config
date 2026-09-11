#include <stdio.h>
#include <unistd.h>
#include "/mnt/code-projects/packages/auto-config/version 3.0/config.h"

void run_plex(void)
{
  run_domain_name();

  snprintf(avail_path, sizeof(avail_path), "/etc/nginx/sites-available/plex");
  FILE *fp = fopen(avail_path, "w");

  fprintf(fp,
    "server {\n"
    "  listen 80;\n"
    "  server_name %s;\n\n"
    "  location / {\n"
    "    proxy_pass http://127.0.0.1:3400;\n"
    "    proxy_http_version 1.1;\n"
    "    proxy_set_header Upgrade $http_upgrade;\n"
    "    proxy_set_header Connection 'upgrade';\n"
    "    proxy_set_header Host $host;\n"
    "    proxy_cache_bypass $http_upgrade;\n"
    "    proxy_set_header X-Real-IP $remote_addr;\n"
    "  }\n\n"
    "  error_page 502 /502.html;\n"
    "  location = /502.html {\n"
    "    root /home/PlexStore;\n"
    "  }\n"
    "}\n",
    domain_name
  );

  fclose(fp);

  snprintf(enabled_path, sizeof(enabled_path), "/etc/nginx/sites-enabled/plex");
  symlink(avail_path, enabled_path);

  certbot();

  return;
}