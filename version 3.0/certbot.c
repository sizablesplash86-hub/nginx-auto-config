#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "config.h"

void certbot(void)
{
  snprintf(certbot_cmd, sizeof(certbot_cmd), "sudo certbot --nginx -d %s", domain_name);

  if (system(certbot_cmd) != 0)
  {
    char rem;
    printf("\033[31mERROR\033[0m SSL certificate failed to deploy\n\n");
    printf("Would you like to remove broken config? (y/n): ");
    scanf(" %c", &rem);

    if (rem == 'y')
    {
      printf("Removing configuration...\n\n");
      unlink(avail_path);
      unlink(enabled_path);
      system("sudo nginx -t");
      return;
    }
    
    if (rem == 'n' || rem == 'q')
    {
      printf("Config not removed. Exiting now...\n\n");
      return;
    }
  }
}
