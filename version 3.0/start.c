#include <stdio.h>
#include <unistd.h>
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

void update_check()
{
  printf("\nupdates not setup yet\n\n");
  return;
}

int root_check()
{
  if (geteuid() != 0)
  {
    printf("\n\033[31mEnter root first\033[0m\n\n");
    return 1;
  }
  return 0;
}

int main()
{
  char one;
  if (root_check() != 0) return 0;
  update_check();

  printf("\nwelcome to N.A.P. %s!\n\n", CURRENT_VERSION);
  
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