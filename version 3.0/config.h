#ifndef CONFIG_H
#define CONFIG_H

#define CURRENT_VERSION "v3.0.1" //skipped making more v2.0 versions. As I was working on v2 and trying to figure out the GUI, I realized starting fresh with a restructure of the code would be best
#define NGINX_VERSION "3.0"
#define PINGORA_VERSION "1.0"
#define REPO_URL "https://api.github.com/repos/sizablesplash86-hub/nginx-auto-config/releases/latest"
#define STR_LEN 256

extern char domain_name[STR_LEN];  // I wanna say somewhere is that the difference between char and int is that char uses less memory
extern char config_name[STR_LEN];
extern char proxy[STR_LEN];
extern char directory[STR_LEN];
extern char php_ver[STR_LEN];
extern char avail_path[STR_LEN];
extern char enabled_path[STR_LEN];
extern char certbot_cmd[STR_LEN];
extern char lan_ip[STR_LEN];

void run_presets(void);
void run_manual(void);
void run_jellyfin(void);
void run_plex(void);
void run_nextcloud(void);
void run_gui(void);
void run_proxy(void);
void run_directory(void);
void certbot(void);
void run_domain_name(void);
void install_gui(void);
void find_ip(void);
void update(void);
void pingora(void);
void nginx(void);
int handle_json_mode(const char *json_filepath);

#endif
