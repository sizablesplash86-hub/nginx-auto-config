#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "config.h"

void update_check()
{
  printf("\nupdates not setup yet\n\n");
  return;
}

int root_check()
{
  if (geteuid() != 0)
  {
    printf("not in root\n\n");
    return 1;
  }
  //printf("\nroot check not setup yet\n\n");
  printf("in root\n\n");
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