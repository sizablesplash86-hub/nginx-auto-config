#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "config.h"

void run_proxy(void)
{
  run_domain_name();

  printf("Input config name: ");
  fgets(config_name, sizeof(config_name), stdin);
  config_name[strcspn(config_name, "\n")] = 0;

  printf("Input proxy port: ");
  fgets(proxy, sizeof(proxy), stdin);
  proxy[strcspn(proxy, "\n")] = 0;

  snprintf(avail_path, sizeof(avail_path), "/etc/nginx/sites-available/%s", config_name);
  FILE *fp = fopen(avail_path, "w");

  fprintf(fp,
    "server {\n"
    "  server_name %s;\n\n"
    "  location / {\n"
    "    proxy_pass http://127.0.0.1:%s;\n"
    "    proxy_set_header Host $host;\n"
    "    proxy_set_header X-Real-IP $remote_addr;\n"
    "    proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;\n"
    "    proxy_set_header X-Forwarded-Proto $scheme;\n"
    "    proxy_set_header X-Forwarded-Protocol $scheme;\n"
    "    proxy_set_header X-Forwarded-Host $http_host;\n"
    "    proxy_http_version 1.1;\n"
    "    proxy_set_header Upgrade $http_upgrade;\n"
    "    proxy_set_header Connection \"upgrade\";\n"
    "  }\n"
    "}\n",
    domain_name, proxy
  );

  fclose(fp);

  snprintf(enabled_path, sizeof(enabled_path), "/etc/nginx/sites-enabled/%s", config_name);
  symlink(avail_path, enabled_path);

  certbot();

  return;
}