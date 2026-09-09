#include <stdio.h>
#include "config.h"

void run_manual(void)
{
  /*
  int c;
  while ((c = getchar()) != '\n' && c != EOF);  */

  char ent;
  printf("Input 1 for proxy or 2 for directory: ");
  scanf(" %c", &ent);

  if (ent == '1')
  {
    run_proxy();
  }

  if (ent == '2')
  {
    run_directory();
  }
  
  return;
}