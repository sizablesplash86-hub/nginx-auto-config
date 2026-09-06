#include <stdio.h>
#include <string.h>
#include "config.h"

void update_check()
{
  printf("\nupdates not setup yet\n\n");
  return;
}

int root_check()
{
  printf("\nroot check not setup yet\n\n");
}

int main()
{
  char one;
  update_check();
  root_check();

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