#include <stdio.h>
#include <stdlib.h>
#include "/mnt/code-projects/packages/auto-config/version 3.0/config.h"

void nginx(void)
{
  char one;
  printf("\nwelcome to the NGINX Auto Config %s!\n\n", NGINX_VERSION);

  if (system("ls -d /etc/nginx/sites-enabled/auto-config-gui > /dev/null 2>&1") == 0)
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
}
