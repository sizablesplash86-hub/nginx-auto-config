#include <stdio.h>
#include <string.h>
#include "config.h"

void run_domain_name(void)
{
  int c;
  while ((c = getchar()) != '\n' && c != EOF);

  printf("Enter domain name: ");
  fgets(domain_name, sizeof(domain_name), stdin);
  domain_name[strcspn(domain_name, "\n")] = 0;

  return;
}