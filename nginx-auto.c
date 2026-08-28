//NGINX auto config
//sources from C Programming A Modern Approach Second Edition by: K. N. King
#include <stdio.h>
#include <unistd.h> //not in the book, wrote it down on page 748 & notes page tho
#include <float.h> //page 589 chapter 23.1
#include <string.h> //chapter 23.6 page 615 appendix 785
#include <stdlib.h> //chapter 26.2 682
#define CURRENT_VERSION "v1.0.2"
#define REPO_URL "https://api.github.com/repos/sizablesplash86-hub/nginx-auto-config/releases/latest"

void check_for_updates(void) {
    char command[512];
    char latest_version[64] = {0};

    // Use curl to fetch the "tag_name" field from GitHub's API (User-Agent added so GitHub doesn't block it)
    snprintf(command, sizeof(command),
        "curl -s -H \"User-Agent: auto-config-app\" %s | grep '\"tag_name\":' | sed -E 's/.*\"([^\"]+)\".*/\\1/'",
        REPO_URL
    );

    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        return; // Skip update check if curl fails
    }

    if (fgets(latest_version, sizeof(latest_version), fp) != NULL) {
        // Strip trailing newline character
        latest_version[strcspn(latest_version, "\r\n")] = '\0';
    }
    pclose(fp);

    // If a tag was returned and it does not match our current version
    if (strlen(latest_version) > 0 && strcmp(latest_version, CURRENT_VERSION) != 0) {
        printf("=========================================\n");
        printf("Update detected! (%s -> %s)\n", CURRENT_VERSION, latest_version);
        printf("Would you like to upgrade? (y/n): ");

        char ans;
        scanf(" %c", &ans);
        while (getchar() != '\n'); // Clear input buffer

        if (ans == 'y' || ans == 'Y') {
            printf("\nDownloading and installing latest package...\n");

            // Construct download and install command for your latest .deb package
            char update_cmd[1024];
            snprintf(update_cmd, sizeof(update_cmd),
                "curl -sL https://github.com/sizablesplash86-hub/nginx-auto-config/releases/download/%s/auto-config_%s_amd64.deb -o /tmp/auto-config_update.deb && "
                "apt install -y /tmp/auto-config_update.deb && "
                "rm /tmp/auto-config_update.deb",
                latest_version, latest_version + 1 // +1 skips the 'v' prefix for deb file name (e.g., 1.0.2)
            );

            int res = system(update_cmd);
            if (res == 0) {
                printf("\nUpgrade completed successfully! Please restart the app.\n");
                exit(0);
            } else {
                printf("\nUpgrade failed. Continuing with current version...\n\n");
            }
        }
    }
}

#define STR_LEN 256 //chapter 13.2 page 281
char config_name[STR_LEN];
char domain_name[STR_LEN];
char target[STR_LEN];

int main()

{
  check_for_updates();
  printf("\nWelcome to the NGINX auto config version 1.0!\n\n");

  printf("Before you continue, make sure you are in root. Press enter to scan: ");

  getchar(); //mentioned in chapter 7.3 on page 148

  if (geteuid() == 0) //not in book, written down on notes page under <unistd.h>
  {
    printf("Root system detected. Press enter to continue: ");
    getchar();

    printf("\nDependencies (certbot & NGINX) need to be installed. Press enter to install: ");
    getchar();

    printf("scanning for system dependencies (nginx & certbot)\n\n");
    if (system("command -v nginx > /dev/null 2>&1 && command -v certbot > /dev/null 2>&1") == 0)
    {
       printf("NGINX and Certbot are already installed! Skipping install.\n");
    }
    else
    { 
      printf("NGINX & CERTBOT NOT INSTALLED! Installing now...\n");
      system("apt update && apt install -y nginx certbot python3-certbot-nginx");
    }
    printf("press enter to continue: ");
    getchar();

    printf("Enter name of config: ");
    fgets(config_name, sizeof(config_name), stdin); //fgets in chapters 13 & 22, appendix 761, stdin on page 541
    config_name[strcspn(config_name, "\n")] = 0; //chapter 23.6
    
    printf("Enter proxy number (ex: 8096): ");
    fgets(target, sizeof(target), stdin); 
    target[strcspn(target, "\n")] = 0;

    printf("Enter your domain name: ");
    fgets(domain_name, sizeof(domain_name), stdin);
    domain_name[strcspn(domain_name, "\n")] = 0;

    printf("Press enter to create the config: ");
    getchar();

    char avail_path[STR_LEN];
    snprintf(avail_path, sizeof(avail_path), "/etc/nginx/sites-available/%s", config_name); //chapter 22.8 page 576

    FILE *fp = fopen(avail_path, "w"); //chapter 22.2 page 545
    if (fp == NULL)
    {
      perror("FILE COULD NOT BE OPENED");
      return 1;
    }
    fprintf(fp, //chapter 22.3 page 552
      "server {\n"
      "  server_name %s;\n\n"
      "  location / {\n"
      "    proxy_pass http://127.0.0.1:%s;\n"
      "    proxy_set_header Host $host;\n"
      "    proxy_set_header X-Real-IP $remote_addr;\n"
      "    proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;\n"
      "    proxy_set_header X-Forwarded-Proto $scheme;\n"
      "    proxy_set_header X-Forwarded-Protocol $scheme;\n"
      "    proxy_set_header X-Forwarded-Host $http_host;\n"
      "    proxy_http_version 1.1;\n"
      "    proxy_set_header Upgrade $http_upgrade;\n"
      "    proxy_set_header Connection \"upgrade\";\n"
      "  }\n"
      "}\n",
      domain_name, target
    );

    fclose(fp);

    char enabled_path[STR_LEN];
    snprintf(enabled_path, sizeof(enabled_path), "/etc/nginx/sites-enabled/%s", config_name);

    // Syntax: symlink(target_file, link_path)
    if (symlink(avail_path, enabled_path) != 0) 
    {
      perror("Failed to create symlink");
    }

    // Verify configuration syntax and rollback if invalid
    if (system("nginx -t") != 0)
    {
      unlink(avail_path);   // Deletes /etc/nginx/sites-available/CONFIGNAME
      unlink(enabled_path); // Deletes /etc/nginx/sites-enabled/CONFIGNAME
      printf("Unknown error occurred. Config rollbacked.\n");
      return 1;
    }

    char certbot_cmd[STR_LEN * 2];
    snprintf(certbot_cmd, sizeof(certbot_cmd), "certbot --nginx -d %s", domain_name);
    system(certbot_cmd);

  }
  else
  {
    printf("ROOT SYSTEM NOT DETECTED! Run the command 'su' or use 'sudo' before attempting again\n\n");
  }
  return 0;
}
