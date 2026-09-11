#include <stdio.h>
#include "/mnt/code-projects/packages/auto-config/version 3.0/config.h"

void run_presets(void)
{
  char two;
  printf("Input 1 for Jellyfin, 2 for Plex, 3 for Nextcloud LEMP Stacks, 4 for auto config GUI: ");
  scanf(" %c", &two);

  if (two == '1')
  {
    run_jellyfin();
  }

  if (two =='2')
  {
    run_plex();
  }

  if (two == '3')
  {
    run_nextcloud();
  }

  if (two == '4')
  {
    run_gui();
  }

  if (two == '5')
  {
    install_gui();
  }

  return;
}