//NGINX auto config
//sources from C Programming A Modern Approach Second Edition by: K. N. King
#include <stdio.h> //standard functions
#include <unistd.h> //not in the book, wrote it down on page 748 & notes page tho
#include <float.h> //page 589 chapter 23.1
#include <string.h> //chapter 23.6 page 615 appendix 785
#include <stdlib.h> //chapter 26.2 682
#include <ctype.h> //chapter 7 somewhere
#include "config.h" //chapter 15.2 p.350

#define CURRENT_VERSION "v1.1.4"
#define REPO_URL "https://api.github.com/repos/sizablesplash86-hub/nginx-auto-config/releases/latest"
#define STR_LEN 256

char config_name[STR_LEN];
char domain_name[STR_LEN];
char target[STR_LEN];
char config_type_input[STR_LEN];
char preset[STR_LEN];
char preset_choice[STR_LEN];
char php_ver[STR_LEN];

void get_nginx_test_error(char *buffer, size_t max_len)
{
    // Redirect stderr (2) to stdout (1) so popen reads Nginx test errors
    FILE *fp = popen("sudo nginx -t 2>&1", "r");
    if (fp == NULL)
    {
        snprintf(buffer, max_len, "Failed to run nginx -t command.");
        return;
    }

    size_t bytes_read = fread(buffer, 1, max_len - 1, fp);
    buffer[bytes_read] = '\0'; // Ensure string is null-terminated

    pclose(fp);
}

void check_for_updates(void) {
    char command[512];
    char latest_version[64] = {0};
    

    snprintf(command, sizeof(command),
        "curl -s -H \"User-Agent: auto-config-app\" %s | grep '\"tag_name\":' | sed -E 's/.*\"([^\"]+)\".*/\\1/'",
        REPO_URL
    );

    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        return; 
    }

    if (fgets(latest_version, sizeof(latest_version), fp) != NULL) {
        latest_version[strcspn(latest_version, "\r\n")] = '\0';
    }
    pclose(fp);

    if (strlen(latest_version) > 0 && strcmp(latest_version, CURRENT_VERSION) != 0) {
        printf("=========================================\n");
        printf("Update detected! (%s -> %s)\n", CURRENT_VERSION, latest_version);
        printf("Would you like to upgrade? (y/n): ");

        char ans;
        scanf(" %c", &ans);
        while (getchar() != '\n');

        if (ans == 'y' || ans == 'Y') {
            printf("\nDownloading and installing latest package...\n");

            /*char update_cmd[1024];
            snprintf(update_cmd, sizeof(update_cmd),
              "curl -sfL https://github.com/sizablesplash86-hub/nginx-auto-config/releases/download/%s/auto-config_%s_amd64.deb -o /tmp/auto-config_update.deb && "
              "dpkg -i /tmp/auto-config_update.deb || apt-get install -f -y && "
              "rm -f /tmp/auto-config_update.deb",
              latest_version, latest_version + 1
            );*/

            const char *ver_num = (latest_version[0] == 'v') ? latest_version + 1 : latest_version;
            char update_cmd[1024];
            snprintf(update_cmd, sizeof(update_cmd),
                "curl -sfL https://github.com/sizablesplash86-hub/nginx-auto-config/releases/download/%s/auto-config_%s_amd64.deb -o /tmp/auto-config_update.deb && "
                "dpkg -i /tmp/auto-config_update.deb || apt-get install -f -y && "
                "rm -f /tmp/auto-config_update.deb",
                latest_version, ver_num
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

void offer_troubleshooting(const char *error_log)
{
  char ans;
  char usr[STR_LEN];
  printf("Would you like to send a discord notification to me for troubleshooting? (y or n): ");
  scanf(" %c", &ans); //chapter 7.3 page 139

  if (ans == 'y')
  {
    printf("Enter a nickname: ");
    scanf(" %255s", usr);
    
    printf("Sending report to my discord server... If you desire to join, https://discord.gg/HjvadzcdzB\n");

    char clean_log[STR_LEN * 4] = {0};
    int j = 0;
    for (int i = 0; error_log[i] != '\0' && j < (sizeof(clean_log) - 4); i++)
    {
      if (error_log[i] == '"')
      {
        clean_log[j++] = '\\';
        clean_log[j++] = '"';
      }
      else if (error_log[i] == '\n')
      {
        clean_log[j++] = '\\';
        clean_log[j++] = 'n';
      }
      else
      {
        clean_log[j++] = error_log[i];
      }
    }

    char curl_cmd[STR_LEN * 8];
    snprintf(curl_cmd, sizeof(curl_cmd),
      "curl -s -X POST \"https://discord.com/api/v10/channels/%s/messages\" "
      "-H \"Authorization: Bot %s\" "
      "-H \"Content-Type: application/json\" "
      "-d '{\"content\": \"**[Nginx-Auto Error Report]**\\n**User:** %s\\n```\\n%s\\n```\"}' > /dev/null",
      DISCORD_CHANNEL_ID, DISCORD_BOT_TOKEN, usr, clean_log
    );

    int res = system(curl_cmd);

    if (res == 0)
    {
      printf("Report successfully sent!\n");  //add a part for messaging back and forth
    }
    else
    {
      printf("\033[31mERROR\033[0m Message failed to send.\n\n");
    }
  }
  if (ans == 'n')
  {
    printf("Log not sent. Exiting now...\n\n");
  }
}

int main()

{
  check_for_updates();
  printf("\nWelcome to the NGINX auto config version 1.1.4!\n");
//printf("Visit port 3487 in your browser to see the graphical install (IN DEVELOPMENT! NOT READY TO USE!)\n\n");
  printf("This config code is intended to work with my server guide at https://www.sizablesplash.com/server-guide\n\n");

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
      system("sudo apt update && sudo apt install -y nginx certbot python3-certbot-nginx");
    }
    printf("press enter to continue: ");
    getchar();

//  might make a manual config option eventually

    printf("Would you like to select from presets? (Y or N): ");
      fgets(preset, sizeof(preset), stdin);
      preset[strcspn(preset, "\n")] = 0;
        if (strcasecmp(preset, "y") == 0)
        {
          printf("1. for Jellyfin 2. for Nextcloud LEMP stacks: ");
            fgets(preset_choice, sizeof(preset_choice), stdin);
            preset_choice[strcspn(preset_choice, "\n")] = 0;
            
            if (atoi(preset_choice) == 1)
            {
              printf("Enter domain name here: ");
                fgets(domain_name, sizeof(domain_name), stdin);
                domain_name[strcspn(domain_name, "\n")] = 0;
            
              printf("Press enter to create config: ");
              getchar();
              char avail_path[STR_LEN];
              snprintf(avail_path, sizeof(avail_path), "/etc/nginx/sites-available/jellyfin", preset_choice);

              FILE *fp = fopen(avail_path, "w");
              if (fp == NULL)
              {
                perror("FILE COULD NOT BE OPENED");
                return 1;
              }
              if (atoi(preset_choice) == 1)
              {
                fprintf(fp,
                  "server {\n"
                  "  server_name %s;\n\n"
                  "  location / {\n"
                  "    proxy_pass http://127.0.0.1:8096;\n"
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
                  domain_name
                );
              }

              fclose(fp);

              char enabled_path[STR_LEN];
              snprintf(enabled_path, sizeof(enabled_path), "/etc/nginx/sites-enabled/jellyfin", preset_choice);

              if (symlink(avail_path, enabled_path) != 0) 
              {
                perror("\033[31mFailed to create symlink\033[0m");
                
              }

              system("sudo nginx -t");

              if (system("sudo nginx -t") != 0)

              if (system("sudo nginx -t") != 0)
              {
                char nginx_error[1024] = {0};

                get_nginx_test_error(nginx_error, sizeof(nginx_error));

                unlink(avail_path);
                unlink(enabled_path);
                printf("\033[31mUnknown error occurred.\033[0m Config rollbacked.\n\n");
                system("sudo nginx -t");
                offer_troubleshooting(nginx_error);
                return 0;
              }

              char certbot_cmd[STR_LEN * 2];
              snprintf(certbot_cmd, sizeof(certbot_cmd), "sudo certbot --nginx -d %s", domain_name);
              system(certbot_cmd);
              system("sudo nginx -t");
              system("sudo systemctl reload nginx");
              system("sudo nginx -t");

              printf("Cert successfully generated! Press enter to exit: ");
              getchar();
            }

            if (atoi(preset_choice) == 2)
            { 
              printf("WARNING!! Make sure you followed the guide in https://www.sizablesplash.com/server-guide/prereqs/OS-config/nextcloud/LEMP-stacks before following these steps. Press enter to continue: ");              
              getchar();

              printf("Verifying PHP version...\n");

              char php_ver[16] = {0};
              char php_sock[STR_LEN] = {0};

              FILE *cmd = popen("php -r 'echo PHP_MAJOR_VERSION.\".\".PHP_MINOR_VERSION;' 2>/dev/null", "r");
              if (cmd != NULL)
              {
                  if (fgets(php_ver, sizeof(php_ver), cmd) != NULL)
                  {
                      php_ver[strcspn(php_ver, "\n")] = 0;
                  }
                  pclose(cmd);
              }
              if (strlen(php_ver) > 0)
              {
                snprintf(php_sock, sizeof(php_sock), "/run/php/php%s-fpm.sock", php_ver);
                printf("PHP %s is installed! -> using socket %s\n", php_ver, php_sock);
              }
              else
              {
                strncpy(php_sock, "/run/php/php-fpm.sock", sizeof(php_sock));
                printf("\033[31mPHP NOT INSTALLED!!!\033[0m Exiting... %s\n", php_sock);
                return 0;
              }

              printf("Enter domain name here: ");
                fgets(domain_name, sizeof(domain_name), stdin);
                domain_name[strcspn(domain_name, "\n")] = 0;
            
              printf("Press enter to create config: ");
              getchar();
              char avail_path[STR_LEN];
              snprintf(avail_path, sizeof(avail_path), "/etc/nginx/sites-available/nextcloud", preset_choice);

              FILE *fp = fopen(avail_path, "w");
              if (fp == NULL)
              {
                perror("\033[31mFILE COULD NOT BE OPENED\033[0");
                return 1;
              }
              if (atoi(preset_choice) == 2)
              {
                fprintf(fp,
                  "  upstream php-handler {\n"
                  "  server unix:%s;\n"
                  "}\n\n"
                  "map $arg_v $asset_immutable {\n"
                  "  \"\" \"\";\n"
                  "  default \", immutable\";\n"
                  "}\n\n"
                  "server {\n"
                  "  listen 80;\n"
                  "  listen [::]:80;\n"
                  "  server_name %s;\n"
                  "  server_tokens off;\n"
                  "  return 301 https://$server_name$request_uri;\n"
                  "}\n\n"
                  "server {\n"
                  "  http2 on;\n"
                  "  server_name %s;\n"
                  "  root /var/www/nextcloud;\n"
                  "  server_tokens off;\n\n"
                  "  client_max_body_size 512M;\n"
                  "  client_body_timeout 300s;\n"
                  "  fastcgi_buffers 64 4K;\n\n"
                  "  gzip on;\n"
                  "  gzip_vary on;\n"
                  "  gzip_comp_level 4;\n"
                  "  gzip_min_length 256;\n"
                  "  gzip_proxied expired no-cache no-store private no_last_modified no_etag auth;\n"
                  "  gzip_types application/atom+xml text/javascript application/javascript application/json application/ld+json application/manifest+json application/rss+xml application/vnd.geo+json application/vnd.ms-fontobject application/wasm application/x-font-ttf application/x-web-app-manifest+json application/xhtml+xml application/xml font/opentype image/bmp image/svg+xml image/x-icon text/cache-manifest text/css text/plain text/vcard text/vnd.rim.location.xloc text/vtt text/x-component text/x-cross-domain-policy;\n\n"
                  "  client_body_buffer_size 512k;\n\n"
                  "  add_header Referrer-Policy \"no-referrer\" always;\n"
                  "  add_header X-Content-Type-Options \"nosniff\" always;\n"
                  "  add_header X-Frame-Options \"SAMEORIGIN\" always;\n"
                  "  add_header X-Permitted-Cross-Domain-Policies \"none\" always;\n"
                  "  add_header X-Robots-Tag \"noindex, nofollow\" always;\n"
                  "  add_header Permissions-Policy \"camera=(), microphone=(), geolocation=()\" always;\n"
                  "  add_header Content-Security-Policy \"default-src 'self'; script-src 'self' 'unsafe-inline' 'unsafe-eval'; style-src 'self' 'unsafe-inline'; img-src 'self' data: blob:; font-src 'self'; connect-src 'self'; frame-ancestors 'self'\" always;\n\n"
                  "  fastcgi_hide_header X-Powered-By;\n\n"
                  "  include mime.types;\n"
                  "  types {\n"
                  "    text/javascript mjs;\n"
                  "    application/wasm wasm;\n"
                  "  }\n\n"
                  "  index index.php index.html /index.php$request_uri;\n\n"
                  "  location = / {\n"
                  "    if ( $http_user_agent ~ ^DavClnt ) {\n"
                  "      return 302 /remote.php/webdav/$is_args$args;\n"
                  "    }\n"
                  "  }\n\n"
                  "  location = /robots.txt {\n"
                  "    allow all;\n"
                  "    log_not_found off;\n"
                  "    access_log off;\n"
                  "  }\n\n"
                  "  location ^~ /.well-known {\n"
                  "    location = /.well-known/carddav { return 301 /remote.php/dav/; }\n"
                  "    location = /.well-known/caldav { return 301 /remote.php/dav/; }\n"
                  "    location /.well-known/acme-challenge { try_files $uri $uri/ =404; }\n"
                  "    location /.well-known/pki-validation { try_files $uri $uri/ =404; }\n"
                  "    return 301 /index.php$request_uri;\n"
                  "  }\n\n"
                  "  location ~ ^/(?:build|tests|config|lib|3rdparty|templates|data)(?:$|/) { return 404; }\n"
                  "  location ~ ^/(?:\\.|autotest|occ|issue|indie|db_|console) { return 404; }\n\n"
                  "  location ~ ^/(?:composer\\.(?:json|lock)|package(?:-lock)?\\.json|core/shipped\\.json)$ {\n"
                  "    return 404;\n"
                  "  }\n\n"
                  "  rewrite ^/(?!index|remote|public|cron|status|ocs\\/v[12]|ocs-provider\\/.+|core\\/ajax\\/update|updater\\/.+|.+\\/richdocumentscode(_arm64)?\\/proxy) /index.php$request_uri;\n\n"
                  "  fastcgi_split_path_info ^(.+?\\.php)(/.*)$;\n"
                  "  set $path_info $fastcgi_path_info;\n\n"
                  "  try_files $fastcgi_script_name =404;\n\n"
                  "  include fastcgi_params;\n"
                  "  fastcgi_pass php-handler;\n\n"
                  "  fastcgi_param SCRIPT_FILENAME $document_root$fastcgi_script_name;\n"
                  "  fastcgi_param PATH_INFO $path_info;\n"
                  "  fastcgi_param HTTPS on;\n"
                  "  fastcgi_param modHeadersAvailable true;\n"
                  "  fastcgi_param front_controller_active true;\n"
                  "  fastcgi_max_temp_file_size 0;\n\n"
                  "  location ~ \\.(?:css|js|mjs|svg|gif|ico|jpg|png|webp|wasm|tflite|map|ogg|flac|mp4|webm)$ {\n"
                  "    try_files $uri /index.php$request_uri;\n"
                  "    add_header Cache-Control \"public, max-age=15778463$asset_immutable\" always;\n"
                  "    add_header Referrer-Policy \"no-referrer\" always;\n"
                  "    add_header X-Content-Type-Options \"nosniff\" always;\n"
                  "    add_header X-Frame-Options \"SAMEORIGIN\" always;\n"
                  "    add_header X-Permitted-Cross-Domain-Policies \"none\" always;\n"
                  "    add_header X-Robots-Tag \"noindex, nofollow\" always;\n"
                  "    access_log off;\n"
                  "  }\n\n"
                  "  location ~ \\.(otf|woff2?)$ {\n"
                  "    try_files $uri /index.php$request_uri;\n"
                  "    expires 7d;\n"
                  "    access_log off;\n"
                  "  }\n\n"
                  "  location /remote {\n"
                  "    return 301 /remote.php$request_uri;\n"
                  "  }\n\n"
                  "  location / {\n"
                  "    try_files $uri $uri/ /index.php$request_uri;\n"
                  "  }\n"
                  "}\n",
                  php_sock,
                  domain_name,
                  domain_name
                );
              }

              fclose(fp);

              char enabled_path[STR_LEN];
              snprintf(enabled_path, sizeof(enabled_path), "/etc/nginx/sites-enabled/nextcloud", preset_choice);

              if (symlink(avail_path, enabled_path) != 0) 
              {
                perror("\033[31mFailed to create symlink\033[0m");
              }

              system("sudo nginx -t");
    
              if (system("sudo nginx -t") != 0)
              {
                char nginx_error[1024] = {0};

                get_nginx_test_error(nginx_error, sizeof(nginx_error));

                unlink(avail_path);
                unlink(enabled_path);
                printf("\033[31mUnknown error occurred.\033[0m Config rollbacked.\n\n");
                system("sudo nginx -t");
                offer_troubleshooting(nginx_error);
                return 0;
              }

              char certbot_cmd[STR_LEN * 2];
              snprintf(certbot_cmd, sizeof(certbot_cmd), "sudo certbot --nginx -d %s", domain_name);
              system(certbot_cmd);
              system("sudo nginx -t");
              system("sudo systemctl reload nginx");
              system("sudo nginx -t");

              printf("Cert successfully generated! Press enter to exit: ");
              getchar();

            }
            //where the presets end
            return 0;
          }
        
    if (strcasecmp(preset, "n") == 0)
    {
      
      printf("Enter name of config: ");
        fgets(config_name, sizeof(config_name), stdin); //fgets 22.5 p.570 appendix 761 and stdin chapter 22.1 page 541 & 570 not in appendix
        config_name[strcspn(config_name, "\n")] = 0; //chapter 23.6 appendix p.784
      
      int config_type = 0;
      while (config_type != 1 && config_type != 2)
      {
        printf("Enter type of config (1 for Reverse Proxy, 2 for Directory/Static Site): ");
        fgets(config_type_input, sizeof(config_type_input), stdin);
        config_type = atoi(config_type_input);

        if (config_type != 1 && config_type != 2)
        {
          printf("Invalid choice. Please enter 1 or 2.\n");
        }
      }

      if (config_type == 1)
      {
        printf("Enter proxy port (ex: 8096): ");
        fgets(target, sizeof(target), stdin); 
        target[strcspn(target, "\n")] = 0;
      }
      else
      {
        printf("Enter root directory path (ex: /var/www/html): ");
        fgets(target, sizeof(target), stdin); 
        target[strcspn(target, "\n")] = 0;
      } 

      printf("Enter your domain name: ");
      fgets(domain_name, sizeof(domain_name), stdin);
      domain_name[strcspn(domain_name, "\n")] = 0;

      printf("Press enter to create the config: ");
      getchar();

      char avail_path[STR_LEN];
      snprintf(avail_path, sizeof(avail_path), "/etc/nginx/sites-available/%s", config_name);

      FILE *fp = fopen(avail_path, "w");
      if (fp == NULL)
      {
        perror("FILE COULD NOT BE OPENED");
        return 1;
      }

      if (config_type == 1)
      {
          fprintf(fp,
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
      }
      else
      {
          fprintf(fp,
            "server {\n"
            "  server_name %s;\n\n"
            "  root %s;\n"
            "  index index.html index.htm;\n\n"
            "  location / {\n"
            "    try_files $uri $uri/ $uri/index.html $uri.html =404;\n"
            "  }\n"
            "}\n",
            domain_name, target
          );
      }

      fclose(fp);

      char enabled_path[STR_LEN];
      snprintf(enabled_path, sizeof(enabled_path), "/etc/nginx/sites-enabled/%s", config_name);

      if (symlink(avail_path, enabled_path) != 0) 
      {
        perror("Failed to create symlink");
      }

      system("sudo nginx -t");

      if (system("sudo nginx -t") != 0)
      {
        char nginx_error[1024] = {0};

        get_nginx_test_error(nginx_error, sizeof(nginx_error));

        unlink(avail_path);
        unlink(enabled_path);
        printf("\033[31mUnknown error occurred.\033[0m Config rollbacked.\n\n");
        system("sudo nginx -t");
        offer_troubleshooting(nginx_error);
        return 0;
      }

      char certbot_cmd[STR_LEN * 2];
      snprintf(certbot_cmd, sizeof(certbot_cmd), "sudo certbot --nginx -d %s", domain_name);
      system(certbot_cmd);
      system("sudo systemctl reload nginx");

      printf("Certificate successfully deployed! Exiting now...\n");
    }
    else
    {
      printf("\033[31mERROR\033[0m run auto-config again and enter correct variable\n\n"); //add in probably a v2.0 to have it work better cuz I don't feel like adding all the code needed
    }
  }
  else
  {
    printf("ROOT SYSTEM NOT DETECTED! Run the command 'su' before attempting again\n\n");
  }
  return 0;
}
