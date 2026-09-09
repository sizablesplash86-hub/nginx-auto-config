#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "config.h"

void run_gui(void)
{
  char warn;
  
  printf("\033[31mWARNING!!\033[0m Opening the auto config GUI to the public internet is risky. Do you wish to proceed? (y/n): ");
  scanf(" %c", &warn);

  if (warn == 'y')
  {
    run_domain_name();

    system("rm /etc/nginx/sites-available/auto-config && rm /etc/nginx/sites-enabled/auto-config");

    snprintf(avail_path, sizeof(avail_path), "/etc/nginx/sites-available/auto-config");
    FILE *fp = fopen(avail_path, "w");
    fprintf(fp,
"server {\n"
        "  listen 80;\n"
        "  listen [::]:80;\n\n"
        "  server_name %s;\n\n"
        "  root /var/www/auto-config;\n"
        "  index index.html index.htm index.php;\n\n"
        "  location / {\n"
        "    autoindex on;\n"
        "    try_files $uri $uri/ $uri/index.html $uri.html $uri.php =404;\n"
        "  }\n"
        "}\n",
        domain_name
    );

    fclose(fp);

    snprintf(enabled_path, sizeof(enabled_path), "/etc/nginx/sites-enabled/auto-config");
    symlink(avail_path, enabled_path);

    certbot();

    return;
  }

  else
  {
    printf("Exiting now...\n\n");
    return;
  }

  return;
}