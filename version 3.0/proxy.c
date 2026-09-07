#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "config.h"

void run_proxy(void)
{
  printf("Input domain name: ");
  fgets(domain_name, sizeof(domain_name), stdin);
  domain_name[strcspn(domain_name, "\n")] = 0;

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

  if (system("sudo nginx -t") != 0)
  {
    printf("\033[31mUNKNOWN ERROR OCCURRED\033[0m\n");
    //printf("Would you like to save a log? (y or n): ");
    printf("Removing broken configuration...\n");
    unlink(avail_path);
    unlink(enabled_path);
    system("sudo nginx -t");
    return;
  }

  //remove this here
  snprintf(certbot_cmd, sizeof(certbot_cmd), "sudo certbot --nginx -d %s", domain_name);

  if (system(certbot_cmd) != 0)
  {
    char rem;
    printf("\033[31mERROR\033[0m SSL certificate failed to deploy\n\n");
    printf("Would you like to remove broken config? (y/n): ");
    scanf(" %c", &rem);

    if (rem == 'y')
    {
      printf("Removing configuration...\n\n");
      unlink(avail_path);
      unlink(enabled_path);
      system("sudo nginx -t");
      return;
    }
    
    if (rem == 'n' || rem == 'q')
    {
      printf("Config not removed. Exiting now...\n\n");
      return;
    }
  }

  return;
}