#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "config.h"

//change these as needed
//also... work on GUI
void update(void)
{
  char command[512];
  char latest_version[64];

  snprintf(command, sizeof(command), "curl -s H\"User-Agent: auto-config-app\" %s | grep '\"tag_name\":' | sed -E 's/.*\"([^\"]+)\".*/\\1/'", REPO_URL);

    FILE *fp = popen(command, "r");
  if (fp == NULL)
  {
    return;
  }

  if (fgets(latest_version, sizeof(latest_version), fp) != NULL)
  {
    latest_version[strcspn(latest_version, "\r\n")] = '\0';
  }
  pclose(fp);

  if (strlen(latest_version) > 0 && strcmp(latest_version, CURRENT_VERSION) != 0)
  {
    printf("%s --> %s", CURRENT_VERSION, latest_version);
    printf("Would you like to upgrade? (y or n): ");


    char ans;
    scanf(" %c", &ans);

    if (ans == 'y')
    {
      printf("\nUpgrading to %s...\n\n", latest_version);

      const char *ver_num = (latest_version[0] == 'v') ? latest_version + 1 : latest_version;
      char update_cmd[1024];

      //THIS WILL NOT WORK!!!! CHANGE THIS!!!!!!!!!
      snprintf(update_cmd, sizeof(update_cmd),
        "curl -sfL https://github.com/sizablesplash86-hub/nginx-auto-config/releases/download/%s/auto-config_%s_amd64.deb -o /tmp/auto-config_update.deb && "
        "dpkg -i /tmp/auto-config_update.deb || apt-get install -f -y && "
        "rm -f /tmp/auto-config_update.deb",
        latest_version, ver_num
      );

      int res = system(update_cmd);
      if (res == 0)
      {
        printf("\nUpgrade to %s completed successfully!\n\n", latest_version);
        system("auto-config");
        exit(0);
      }
      else
      {
        printf("\nUPGRADE FALIED! Continuing with current version...\n\n");
      }
    }
  }
}