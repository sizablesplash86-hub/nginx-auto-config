#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

char domain_name[256];
char config_name[256];
char proxy[256];
char directory[256];
char php_ver[256];
char avail_path[256];
char enabled_path[256];
char certbot_cmd[256];
char lan_ip[256];

// from here
int main(int argc, char *argv[])
{
  if (argc >= 3 && strcmp(argv[1], "--json") == 0)
  {
    return handle_json_mode(argv[2]);
  } //to here is part of the vibe coded GUI

  char web;
  if (geteuid() != 0)
  {
    printf("\n\033[31mEnter root first\033[0m\n\n");
    return 1;
  }
  update();

  printf("\nwelcome to the auto config %s!\n\n", CURRENT_VERSION);
  
  if (system("ls /etc/nginx/sites-enabled/auto-config-gui > /dev/null 2>&1") == 0)
  {
    find_ip();
    printf("Visit \033[34mhttp://%s:3487\033[0m in your browser to use the GUI\n\n", lan_ip);
  }


  printf("Input your web server. 1 for NGINX 2 for Pingora: ");
  scanf(" %c", &web);
  if (web == '1')
  {
    nginx();
    return 0;
  }

  if (web == '2')
  {
    pingora();
    return 0;
  }

  return 0;
}