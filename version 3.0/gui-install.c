#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "config.h"

void install_gui(void)
{
  printf("Installing dependencies...\n\n");
  system("sudo apt install php-cli php-fpm php-common -y");

  printf("Creating GUI...\n\n");
  system("mkdir /var/www/auto-config");
  
  snprintf(avail_path, sizeof(avail_path), "/etc/nginx/sites-available/auto-config");
  FILE *fp = fopen(avail_path, "w");

  fprintf(fp,
    "server {\n"
    "  listen 3487;\n\n"
    "  server_name _;\n\n"
    "  root /var/www/auto-config;\n"
    "  index index.php index.html index.htm;\n\n"
    "  location / {\n"
    "    autoindex on;\n"
    "    try_files $uri $uri/ =404;\n"
    "  }\n\n"
    "  location ~ \\.php$ {\n"
    "    include snippets/fastcgi-php.conf;\n"
    "    fastcgi_pass unix:/run/php/php-fpm.sock;\n"
    "  }\n"
    "}\n"
  );

  fclose(fp);

  snprintf(enabled_path, sizeof(enabled_path), "/etc/nginx/sites-enabled/auto-config");
  symlink(avail_path, enabled_path);

  if (system("sudo nginx -t") != 0)
  {
    printf("\033[31mUNKNOWN ERROR OCCURRED\033[0m\n");
    printf("GUI failed to install.\n");
    unlink(avail_path);
    unlink(enabled_path);
    system("sudo nginx -t");
    printf("Exiting now...\n\n");
    return;
  }

  system("curl -sSL https://repo.sizablesplash.com/auto-config/gui.html -o /var/www/auto-config/index.html");
  system("curl -sSL https://repo.sizablesplash.com/auto-config/gui.php -o /var/www/auto-config/api.php");

  //test these
  system("echo 'www-data ALL=(ALL) NOPASSWD: /usr/local/bin/nginx-auto' | sudo tee /etc/sudoers.d/nginx-auto > /dev/null");
  system("chmod 0440 /etc/sudoers.d/nginx-auto");

  system("sudo systemctl reload nginx");

  find_ip();
  printf("Visit \033[34mhttp://%s:3487\033[0m to use the GUI\n\n", lan_ip);

  return;
}