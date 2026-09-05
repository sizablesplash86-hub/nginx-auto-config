//sources from K. N. King's C Programming a Modern Aproach Second Edition
//NGINX Auto Program! It's as easy as taking a N.A.P.
#include <stdio.h> //standard functions
#include <unistd.h> //not in the book, wrote down in the back pages
#include <float.h> //not even sure if I still need this, chapter 23 p. 589
#include <string.h> //chapter 23 p. 277
#include <stdlib.h> //chapter 26.2 p. 682
#include <ctype.h> //chapter 23.5 p. 612
/*
#include "config.h" //chapter 15.2 p. 350   no longer needed, I'll leave it tho. It was for a discord bot implimentation in v1 */

#define CURRENT_VERSION "v2.0.0" //version
#define REPO_URL "https://api.github.com/repos/sizablesplash86-hub/nginx-auto-config/releases/latest" //for updates
#define STR_LEN 256

char config_name[STR_LEN]; //chapter 13
char proxy[STR_LEN];
char directory[STR_LEN];
char domain_name[STR_LEN];
char php_ver[STR_LEN];

/*
void update_check(); //figure this out once the outline for the code is reconstructed
{
  //
} */

/* //DO NOT USE THIS, I'M JUST KEEPING FOR REFERENCE
typedef struct {
    char preset[32];
    char name[64];
    char domain[128];
    char port[10];
    char path[256];
    char phpver[10];
} ConfigArgs;

void parse_json_config(const char *filepath, ConfigArgs *config) {
    FILE *f = fopen(filepath, "r");
    if (!f) {
        perror("Failed to open JSON file");
        exit(1);
    }

    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *data = malloc(length + 1);
    fread(data, 1, length, f);
    fclose(f);
    data[length] = '\0';

    // Simple parsing logic or use cJSON library here:
    // cJSON *json = cJSON_Parse(data);
    
    free(data);
} //DO NOT JUST USE THIS, I'M KEEPING IT FOR REFERENCE */

void clear_buffer() //kinda mentioned in chapter 9.2 //reread the section on scanf in chapter 3/4, read chapters 9, 12, & 13  //void functions are in chapter 9 p.183
{
  int c;
  while ((c = getchar()) != '\n' && c != EOF);  //while statements chapter 6.1 p.99
}

int root_check()  //if it starts with void (ex: void root_check() it will not work with return 0; or return 1; it has to start with int
{
  if (geteuid() != 0)  //if statements are covered in chapter 5.2 while geteuid is a command not in the book as it works with the <unistd.h> header
  {
    printf("\n\033[31mEnter root first\033[0m\n\n");  //printf is one of the basic commands of C introduced in the first chapter
    return 1;  //mentioned many times
  } 
  return 0;  //exits code. Mentioned in one of the first pages
}

void get_nginx_test_error(char *buffer, size_t max_len)  //didn't know you could use this for char stuff  //size_t is not a character and is a function mentioned on p.151 chapter 7.6
{
  FILE *fp = popen("sudo nginx -t 2>&1", "r"); //2>&1 is a Linux command that saves the log in an easier to read way  //still don't know the purpose of the "r"    wait... I think R is read and W is write
  fp;
  
  size_t bytes_read = fread(buffer, 1, max_len - 1, fp); //bytes_read is not in the book, wrote it down chapter 7.6 p.151  //fread is in the book chapter 22.6 p.571
  buffer[bytes_read] = '\0';

  pclose(fp);
}

void error_log()
{
  // I'm lazy
}

void lan_ip
{
  
}

int main() //idk what this is officially called, I just know kinda how to use it
{
//update_check();  //work on this after it's all finalized

  if (root_check() != 0)
  {
    return 0;
  }

  printf("\nWelcome to the NGINX Auto Config %s! Now known as N.A.P. for NGINX Auto Program because it's as easy as taking a NAP.\n\n", CURRENT_VERSION);
  
  //printf("Visit 3487 in the browser to use the graphical interface"); //work on this later
  
  printf("This program is designed to work with my server guide \033[34mhttps://www.sizablesplash.com/server-guide\033[0m\n\n");

  printf("at any prompt, input q to exit\n\n");

  char preset; //idk when the char parts get mentioned in the book, the parts I've read use     int example, example2;
  printf("Would you like to pick between presets? (y or n): ");
  scanf(" %c", &preset); //scanf is mentioned in chapter 3.2 

  if (preset == 'y')
  {
    char presets;
    printf("Input 1 for Jellyfin, 2 for Nextcloud LEMP stacks, 3 for Plex, 4 for auto config GUI (NOT RECOMMENDED): ");
    scanf(" %c", &presets);

    if (presets == '1')
    {
      //Jellyfin preset
      clear_buffer();

      printf("Enter domain name: ");
      fgets(domain_name, sizeof(domain_name), stdin); //I give up on citations for now, no one will read these. I know where this is, I'm just lazy
      domain_name[strcspn(domain_name, "\n")] = 0;
      char avail_path[STR_LEN];
      snprintf(avail_path, sizeof(avail_path), "/etc/nginx/sites-available/jellyfin");  //sizeof has a section dedicated to it; chapter 7.6 on page 151
      FILE *fp = fopen(avail_path, "w");
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
      fclose(fp);

      char enabled_path[STR_LEN];
      snprintf(enabled_path, sizeof(enabled_path), "/etc/nginx/sites-enabled/jellyfin");

      symlink(avail_path, enabled_path);  //not in book

      if (system("sudo nginx -t") != 0)
      {
        printf("\033[31mUNKNOWN ERROR OCCURRED\033[0m\n");
        //printf("Would you like to save a log? (y or n): ");
        printf("Removing broken configuration...\n");
        unlink(avail_path);
        unlink(enabled_path);
        system("sudo nginx -t");
        return 0;
      }
      
      char certbot_cmd[STR_LEN * 2];
      snprintf(certbot_cmd, sizeof(certbot_cmd), "sudo certbot --nginx -d %s", domain_name);
      
      if (system(certbot_cmd) !=0)
      {
        char rem;
        printf("\033[31mERROR\033[0m SSL certificate failed to deploy\n\n");
        printf("Would you like to remove config in /etc/nginx/sites-available /etc/nginx/sites-enabled? (y or n): ");
        scanf(" %c", &rem);
        if (rem == 'y')
        {
          printf("Removing broken configuration...\n");
          unlink(avail_path);
          unlink(enabled_path);
          system("sudo nginx -t");
          return 0;
        }
        if (rem == 'n' || rem == 'q')
        {
          printf("Exiting now...\n\n");
          return 0;
        }
      }
      //system(certbot_cmd);
      system("sudo nginx -t");
      system("sudo systemctl reload nginx");
      printf("SSL certificate sucessfully deployed! Visit \033[34mhttps://%s\033[0m in your browser.\n\nExiting now...\n\n", domain_name);
    }
    
    if (presets == '2')
    {
      printf("Make sure to follow my LEMP Stacks guide at \033[34mhttps://www.sizablesplash.com/server-guide/prereqs/OS-config/nextcloud/LEMP-stacks\033[0m\n\n");

      clear_buffer();
      
      char php_ver[STR_LEN];
      char php_sock[STR_LEN];

      FILE *cmd = popen("php -r 'echo PHP_MAJOR_VERSION.\".\".PHP_MINOR_VERSION;' 2>/dev/null", "r"); // popen is not in the book, wrote down in appendix D p.778 // 2>dev/null is a linux command that hides error outputs

      if (cmd == NULL || fgets(php_ver, sizeof(php_ver), cmd) == NULL || strlen(php_ver) == 0)
      {
        if (cmd != NULL)
        {
          pclose(cmd);
        }
        printf("\033[31mPHP not installed.\033[0m Exiting now...\n\n");
        return 0;
      }

    pclose(cmd);

    php_ver[strcspn(php_ver, "\r\n")] = 0;

      snprintf(php_sock, sizeof(php_sock), "/run/php/php%s-fpm.sock", php_ver);

      printf("Enter domain name: ");
      fgets(domain_name, sizeof(domain_name), stdin);
      domain_name[strcspn(domain_name, "\n")] = 0;

      char avail_path[STR_LEN];
      snprintf(avail_path, sizeof(avail_path), "/etc/nginx/sites-available/nextcloud");
      FILE *fp = fopen(avail_path, "w");
      fprintf(fp,
        "upstream php-handler {\n"
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

      fclose(fp);

      char enabled_path[STR_LEN];
      snprintf(enabled_path, sizeof(enabled_path), "/etc/nginx/sites-enabled/nextcloud");

      symlink(avail_path, enabled_path);

      if (system("sudo nginx -t") != 0)
      {
        printf("\033[31mUNKNOWN ERROR OCCURRED\033[0m\n");
        //printf("Would you like to save a log? (y or n): ");
        printf("Removing broken configuration...\n");
        unlink(avail_path);
        unlink(enabled_path);
        system("sudo nginx -t");
        return 0;
      }

      else{}

      char certbot_cmd[STR_LEN *2];
      snprintf(certbot_cmd, sizeof(certbot_cmd), "sudo certbot --nginx -d %s", domain_name);

      if (system(certbot_cmd) !=0)
      {
        char rem;
        printf("\033[31mERROR\033[0m SSL certificate failed to deploy\n\n");
        printf("Would you like to remove config in /etc/nginx/sites-available /etc/nginx/sites-enabled? (y or n): ");
        scanf(" %c", &rem);
        if (rem == 'y')
        {
          printf("Removing broken configuration...\n");
          unlink(avail_path);
          unlink(enabled_path);
          system("sudo nginx -t");
          return 0;
        }
        if (rem == 'n' || rem == 'q')
        {
          printf("Exiting now...\n\n");
          return 0;
        }
      }
      else
      {
        printf("SSL certificate sucessfully deployed! Visit \033[34mhttps://%s\033[0m in your browser.\n\nExiting now...\n\n", domain_name);
        system("sudo nginx -t");
        system("sudo systemctl reload nginx");
      }
    }
    if (presets == '3') //plex
    {
      clear_buffer();

      printf("Enter domain name: ");
      fgets(domain_name, sizeof(domain_name), stdin);
      domain_name[strcspn(domain_name, "\n")] = 0;
      char avail_path[STR_LEN];
      snprintf(avail_path, sizeof(avail_path), "/etc/nginx/sites-available/plex");
      FILE *fp = fopen(avail_path, "w");

      fprintf(fp,
        "server {\n"
        "  listen 80;\n"
        "  server_name %s;\n\n"
        "  location / {\n"
        "    proxy_pass http://127.0.0.1:3400;\n"
        "    proxy_http_version 1.1;\n"
        "    proxy_set_header Upgrade $http_upgrade;\n"
        "    proxy_set_header Connection 'upgrade';\n"
        "    proxy_set_header Host $host;\n"
        "    proxy_cache_bypass $http_upgrade;\n"
        "    proxy_set_header X-Real-IP $remote_addr;\n"
        "  }\n\n"
        "  error_page 502 /502.html;\n"
        "  location = /502.html {\n"
        "    root /home/PlexStore;\n"
        "  }\n"
        "}\n",
        domain_name
      );
      fclose(fp);

      char enabled_path[STR_LEN];
      snprintf(enabled_path, sizeof(enabled_path), "/etc/nginx/sites-enabled/plex");
      symlink(avail_path, enabled_path);

      if (system("sudo nginx -t") != 0)
      {
        printf("\033[31mUNKNOWN ERROR OCCURRED\033[0m\n");
        //printf("Would you like to save a log? (y or n): ");
        printf("Removing broken configuration...\n");
        unlink(avail_path);
        unlink(enabled_path);
        system("sudo nginx -t");
        return 0;
      }

      char certbot_cmd[STR_LEN * 2];
      snprintf(certbot_cmd, sizeof(certbot_cmd), "sudo certbot --nginx -d %s", domain_name);

      if (system(certbot_cmd) != 0)
      {
        char rem;
        printf("\033[31mERROR\033[0m SSL certificate failed to deploy\n\n");
        printf("Would you like to remove config in /etc/nginx/sites-available /etc/nginx/sites-enabled? (y or n): ");
        scanf(" %c", &rem);
        if (rem == 'y')
        {
          printf("Removing broken configuration...\n");
          unlink(avail_path);
          unlink(enabled_path);
          system("sudo nginx -t");
          return 0;
        }
        if (rem == 'n' || rem == 'q')
        {
          printf("Exiting now...\n\n");
          return 0;
        }
      }
      
      system("sudo nginx -t");
      system("sudo systemctl reload nginx");
      printf("SSL certificate successfully deployed! Visit \033[0mhttps://%s\033[0m in you browser.\n\n");

    } //plex ends here

    if (presets == '4') //auto config GUI
    {
      char gui;
      printf("\033[31mWARNING!!\033[0m Opening the auto config GUI to the public internet is risky. Do you wish to proceed? (y or n): ");
      scanf(" %c", &gui);

      if (gui == 'n') return 0;

      clear_buffer();

      printf("Enter domain name: ");
      fgets(domain_name, sizeof(domain_name), stdin);
//    domain_name[strcspn(domain_name, "\n")] = 0;
      
      system("rm /etc/nginx/sites-available/auto-config && rm /etc/nginx/sites-enabled/auto-config");
      char avail_path[STR_LEN];
      snprintf(avail_path, sizeof(avail_path), "/etc/nginx/sites-available/auto-config");
      FILE *fp = fopen(avail_path, "w");

      fprintf(fp,
        "server {\n"
        "  listen 80;\n"
        "  listen [::]:80;\n\n"

        "  server_name %s;\n\n"

        "  root /var/www/auto-config/;\n"
        "  index index.html index.php;\n\n"

        "  location / {\n"
        "    autoindex on;\n"
        "    try_files $uri $uri/ $uri/index.html $uri.html $uri.php =404;\n"
        "  }\n"
        "}\n",
        domain_name
      );

      fclose(fp);

      char enabled_path[STR_LEN];
      snprintf(enabled_path, sizeof(enabled_path), "/etc/nginx/sites-enabled/auto-config");
      symlink(avail_path, enabled_path);

      if (system("sudo nginx -t") != 0)
      {
        printf("\033[31mUNKNOWN ERROR OCCURRED\033[0m\n");
        //printf("Would you like to save a log? (y or n): ");
        printf("Removing broken configuration...\n");
        unlink(avail_path);
        unlink(enabled_path);
        system("sudo nginx -t");
        return 0;
      }

      char certbot_cmd[STR_LEN * 2];
      snprintf(certbot_cmd, sizeof(certbot_cmd), "sudo certbot --nginx -d %s", domain_name);

      if (system(certbot_cmd) !=0)
      {
        char rem;
        printf("\033[31mERROR\033[0m SSL certificate failed to deploy\n\n");
        printf("Would you like to remove config in /etc/nginx/sites-available /etc/nginx/sites-enabled? (y or n): ");
        scanf(" %c", &rem);
        if (rem == 'y')
        {
          printf("Removing broken configuration...\n");
          unlink(avail_path);
          unlink(enabled_path);
          system("sudo nginx -t");
          return 0;
        }
        if (rem == 'n' || rem == 'q')
        {
          printf("Exiting now...\n\n");
          return 0;
        }
      }
      else
      {
        printf("SSL certificate sucessfully deployed! Visit \033[34mhttps://%s\033[0m in your browser.\n\nExiting now...\n\n", domain_name);
        system("sudo nginx -t");
        system("sudo systemctl reload nginx");
      }
    } //GUI ends here
    return 0;
  } //presets end here

  else{}

  if (preset == 'q') return 0;
  
  char type;
  printf("Input 1 for reverse proxy or 2 for standalone directory: ");
  scanf(" %c", &type);

  if (type == '1')
  {
    clear_buffer();
    printf("Enter the proxy port: ");
    fgets(proxy, sizeof(proxy), stdin);
    proxy[strcspn(proxy, "\n")] = 0;
  }
    
  if (type == '2')
  {
    clear_buffer();
    printf("Enter directory path: ");
    fgets(directory, sizeof(directory), stdin);
    directory[strcspn(directory, "\n")] = 0;
  }
    
  printf("Enter name of the config: ");
  fgets(config_name, sizeof(config_name), stdin);
  config_name[strcspn(config_name, "\n")] = 0;

  printf("Enter domain name: ");
  fgets(domain_name, sizeof(domain_name), stdin);
  domain_name[strcspn(domain_name, "\n")] = 0;
   

  char avail_path[STR_LEN];
  snprintf(avail_path, sizeof(avail_path), "/etc/nginx/sites-available/%s", config_name);
  FILE *fp = fopen(avail_path, "w");

  if (type == '1')
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
      domain_name, proxy
    );
  }

  if (type == '2')
  {
    if (system("php -v 1>/dev/null") == 0)
    {
      fprintf(fp,
        "server {\n"
        "  listen 80;\n"
        "  listen [::]:80;\n\n"
        "  server_name %s;\n\n"
        "  root %s;\n"
        "  index index.html index.htm index.php;\n\n"
        "  location / {\n"
        "    autoindex on;\n"
        "    try_files $uri $uri/ $uri/index.html $uri.html $uri.php =404;\n"
        "  }\n"
        "}\n",
        domain_name, directory
      );
    }

    else
    {
      fprintf(fp,
        "server {\n"
        "  listen 80;\n"
        "  listen [::]:80;\n\n"
        "  server_name %s;\n\n"
        "  root %s;\n"
        "  index index.html index.htm;\n\n"
        "  location / {\n"
        "    autoindex on;\n"
        "    try_files $uri $uri/ $uri/index.html $uri.html =404;\n"
        "  }\n"
        "}\n",
        domain_name, directory
      );
    }
  }

  fclose(fp);

  char enabled_path[STR_LEN];
  snprintf(enabled_path, sizeof(enabled_path), "/etc/nginx/sites-enabled/%s", config_name);
  symlink(avail_path, enabled_path);

  if (system("sudo nginx -t") != 0)
  {
    printf("\033[31mUNKNOWN ERROR OCCURRED\033[0m\n");
    //printf("Would you like to save a log? (y or n): ");
    printf("Removing broken configuration...\n");
    unlink(avail_path);
    unlink(enabled_path);
    system("sudo nginx -t");
    return 0;
  }

  char certbot_cmd[STR_LEN * 2];
  snprintf(certbot_cmd, sizeof(certbot_cmd), "sudo certbot --nginx -d %s", domain_name);

  if (system(certbot_cmd) !=0)
  {
    char rem;
    printf("\033[31mERROR\033[0m SSL certificate failed to deploy\n\n");
    printf("Would you like to remove config in /etc/nginx/sites-available /etc/nginx/sites-enabled? (y or n): ");
    scanf(" %c", &rem);
    if (rem == 'y')
    {
      printf("Removing broken configuration...\n");
      unlink(avail_path);
      unlink(enabled_path);
      system("sudo nginx -t");
      return 0;
    }
    if (rem == 'n' || rem == 'q')
    {
      printf("Exiting now...\n\n");
      return 0;
    }
  }
  else
  {
    printf("SSL certificate sucessfully deployed! Visit \033[34mhttps://%s\033[0m in your browser.\n\nExiting now...\n\n", domain_name);
    system("sudo nginx -t");
    system("sudo systemctl reload nginx");
  }
  return 0;
}