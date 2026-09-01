#include <stdio.h>
#include <unistd.h>
#include <float.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define CURRENT_VERSION "v2.0.0"
#define STR_LEN 256

char config_name[STR_LEN];
char proxy[STR_LEN];
char directory[STR_LEN];
char domain_name[STR_LEN];

/*
void update_check();
{
  //
} */

void clear_buffer()
{
  int c;
  while ((c = getchar()) != '\n' && c != EOF);
}

int main()
{
//update_check();
   if (geteuid() != 0)
  {
    printf("\n\033[31mEnter root first\033[0m\n\n");
    return 0;
  }

  printf("\nWelcome to the NGINX Auto Config v2.0! Now known as N.A.P. for NGINX Auto Program because it's as easy as taking a NAP.\n\n");
  
  //printf("Visit 3487 in the browser to use the graphical interface");
  
  printf("This program is designed to work with my server guide https://www.sizablesplash.com/server-guide\n\n");

  printf("at any prompt, input q to exit\n\n");

  char preset;
  printf("Would you like to pick between presets? (y or n): ");
  scanf(" %c", &preset);

  if (preset == 'y')
  {
    printf("Input 1 for Jellyfin or 2 for Nextcloud LEMP stacks.\n");

    printf("NOT SETUP YET!!\n");
    //work on this
    return 0;
  }

  if (preset == 'n')
  {
  }

  if (preset == 'q')
  {
    return 0;
  }
  
  char type;
  printf("Input 1 for reverse proxy or 2 for standalone directory: ");
  scanf(" %c", &type);

  if (type == '1')
  {
    clear_buffer();
    printf("Enter the proxy port: ");
    fgets(proxy, sizeof(proxy), stdin);
    proxy[strcspn(proxy, "\n")] = 0;
  }
    
  if (type == '2')
  {
    clear_buffer();
    printf("Enter directory path: ");
    fgets(directory, sizeof(directory), stdin);
    directory[strcspn(directory, "\n")] = 0;
  }
    
  printf("Enter name of the config: ");
  fgets(config_name, sizeof(config_name), stdin);
  config_name[strcspn(config_name, "\n")] = 0;

  printf("Enter domain name: ");
  fgets(domain_name, sizeof(domain_name), stdin);
  domain_name[strcspn(domain_name, "\n")] = 0;

  char six_seven;
  printf("Are you ready generate config? (y or n): ");
  scanf(" %c", &six_seven);
  
  if (six_seven == 'y')
  {

  }

    if (six_seven == 'n')
  {
    printf("Config not generated. Exiting now...\n\n");
    return 0;
  }

  if (six_seven == 'q')
  {
    return 0;
  }

  char avail_path[STR_LEN];
  snprintf(avail_path, sizeof(avail_path), "/etc/nginx/sites-available/%s", config_name);
   
  FILE *fp = fopen(avail_path, "w");

  if (type == '1')
  {
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
  }

    if (type == '2')
    {
      fprintf(fp,
        "server {\n"
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

    char enabled_path[STR_LEN];
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
    }

    char certbot_cmd[STR_LEN * 2];
    snprintf(certbot_cmd, sizeof(certbot_cmd), "sudo certbot --nginx -d %s", domain_name);
    system(certbot_cmd);
    system("sudo nginx -t");
    system("sudo systemctl reload nginx");
    
    printf("SSL certificate sucessfully deployed! Visit \033[34mhttps://%s\033[0m in your browser.\n\nExiting now...\n\n", domain_name);

  return 0;
}