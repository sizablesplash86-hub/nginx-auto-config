//entirely vibe coded

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "/mnt/code-projects/packages/auto-config/version 3.0/config.h"

int handle_json_mode(const char *json_filepath) {
    FILE *f = fopen(json_filepath, "r");
    if (!f) {
        printf("Error: Unable to open JSON file %s\n", json_filepath);
        return 1;
    }

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *json_buf = malloc(len + 1);
    if (!json_buf) {
        fclose(f);
        return 1;
    }
    fread(json_buf, 1, len, f);
    fclose(f);
    json_buf[len] = '\0';

    // Parse values from JSON string
    sscanf(strstr(json_buf, "\"name\":") ? strstr(json_buf, "\"name\":") : "", "\"name\": \"%255[^\"]\"", config_name);
    sscanf(strstr(json_buf, "\"domain\":") ? strstr(json_buf, "\"domain\":") : "", "\"domain\": \"%255[^\"]\"", domain_name);
    sscanf(strstr(json_buf, "\"port\":") ? strstr(json_buf, "\"port\":") : "", "\"port\": \"%255[^\"]\"", proxy);
    sscanf(strstr(json_buf, "\"path\":") ? strstr(json_buf, "\"path\":") : "", "\"path\": \"%255[^\"]\"", directory);

    free(json_buf);

    if (strlen(config_name) == 0 || strlen(domain_name) == 0) {
        printf("Error: Missing required fields (name or domain) in JSON payload\n");
        return 1;
    }

    // Build NGINX file based on proxy vs directory input
    if (strlen(proxy) > 0) {
        run_proxy();
    } else if (strlen(directory) > 0) {
        run_directory();
    }

    return 0;
}