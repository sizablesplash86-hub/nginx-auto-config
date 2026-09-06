#include <stdio.h>
#include <string.h>
#include "config.h"

void update_check()
{
  printf("\nNot setup yet\n\n");
  return;
}

int root_check()
{
  printf("\nNot setup yet\n\n");
}

int main()
{
  char one;
  update_check();
  root_check();

  printf("\nwelcome to N.A.P. %s!\n\n", CURRENT_VERSION);
  
  printf("Would you like to pick between presets? y/n: ");
  scanf(" %c", &one);

  return 0;
}