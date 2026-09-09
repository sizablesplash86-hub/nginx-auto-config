#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "config.h"

void run_directory(void)
{
  run_domain_name();

  printf("Enter config name: ");
  fgets(config_name, sizeof(config_name), stdin);
  config_name[strcspn(config_name, "\n")] = 0;

  printf("Enter directory: ");
  fgets(directory, sizeof(directory), stdin);
  directory[strcspn(directory, "\n")] = 0;

  snprintf(avail_path, sizeof(avail_path), "/etc/nginx/sites-available/%s", config_name);
  FILE *fp = fopen(avail_path, "w");

  if (system("php -v > /dev/null 2>&1") == 0)
  {
    FILE *cmd = popen("php -r 'echo PHP_MAJOR_VERSION.\".\".PHP_MINOR_VERSION;' 2>/dev/null", "r");

    if (cmd == NULL || fgets(php_ver, sizeof(php_ver), cmd) == NULL || strlen(php_ver) == 0)
    {
      if (cmd != NULL)
      {
        pclose(cmd);
      }
    }

    fprintf(fp,
      "server {\n"
      "  listen 80;\n"
      "  listen [::]:80;\n\n"
      "  server_name %s;\n\n"
      "  root %s;\n"
      "  index index.php index.html index.htm;\n\n"
      "  location / {\n"
      "    autoindex on;\n"
      "    try_files $uri $uri/ =404;\n"
      "  }\n\n"
      "  location ~ \\.php$ {\n"
      "    include snippets/fastcgi-php.conf;\n"
      "    fastcgi_pass unix:/run/php/php%s-fpm.sock;\n"
      "  }\n"
      "}\n",
      domain_name, directory, php_ver
    );
  }

  else
  {
    fprintf(fp,
      "server {\n"
      "  listen 80;\n"
      "  listen [::]:80;\n\n"
      "  server_name %s;\n\n"
      "  root %s;\n"
      "  index index.html index.htm;\n\n"
      "  location / {\n"
      "    autoindex on;\n"
      "    try_files $uri $uri/ $uri/index.html $uri.html =404;\n"
      "  }\n"
      "}\n",
      domain_name, directory
    );
  }

  fclose(fp);

  snprintf(enabled_path, sizeof(enabled_path), "/etc/nginx/sites-enabled");
  symlink(avail_path, enabled_path);

  certbot();

  return;
}