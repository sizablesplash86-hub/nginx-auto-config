#include <stdio.h>
#include <string.h>
#include "config.h"

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
  return;
}