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

int root_check()
{
  if (geteuid() != 0)
  {
    printf("\n\033[31mEnter root first\033[0m\n\n");
    return 1;
  }
  return 0;
}

// from here
int main(int argc, char *argv[])
{
  if (argc >= 3 && strcmp(argv[1], "--json") == 0)
  {
    return handle_json_mode(argv[2]);
  } //to here is part of the vibe coded GUI

  char one;
  if (root_check() != 0) return 0;
  update();

  printf("\nwelcome to N.A.P. %s!\n\n", CURRENT_VERSION);
  
  if (system("ls /var/www/auto-config > /dev/null 2>&1") == 0)
  {
    find_ip();
    printf("Visit \033[34mhttp://%s:3487\033[0m in your browser to use the GUI\n\n", lan_ip);
  }


  printf("Would you like to pick between presets? y/n: ");
  scanf(" %c", &one);
  if (one == 'y')
  {
    run_presets();
  }

  if (one == 'n')
  {
    run_manual();
  }

  return 0;
}