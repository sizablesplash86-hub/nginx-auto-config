#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nginx_auto.h"

int main(int argc, char *argv[]) {
    if (root_check() != 0) return 1;

    if (argc < 3 || strcmp(argv[1], "--json") != 0) {
        printf("Usage: %s --json /path/to/file.json\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[2], "r");
    if (!f) {
        printf("Error: Unable to open JSON file %s\n", argv[2]);
        return 1;
    }

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *json_buf = malloc(len + 1);
    fread(json_buf, 1, len, f);
    fclose(f);
    json_buf[len] = '\0';

    ConfigPayload config = {0};

    sscanf(strstr(json_buf, "\"preset\":") ? strstr(json_buf, "\"preset\":") : "", "\"preset\": \"%255[^\"]\"", config.preset);
    sscanf(strstr(json_buf, "\"name\":") ? strstr(json_buf, "\"name\":") : "", "\"name\": \"%255[^\"]\"", config.name);
    sscanf(strstr(json_buf, "\"domain\":") ? strstr(json_buf, "\"domain\":") : "", "\"domain\": \"%255[^\"]\"", config.domain);
    sscanf(strstr(json_buf, "\"port\":") ? strstr(json_buf, "\"port\":") : "", "\"port\": \"%255[^\"]\"", config.port);
    sscanf(strstr(json_buf, "\"path\":") ? strstr(json_buf, "\"path\":") : "", "\"path\": \"%255[^\"]\"", config.path);
    sscanf(strstr(json_buf, "\"phpver\":") ? strstr(json_buf, "\"phpver\":") : "", "\"phpver\": \"%255[^\"]\"", config.phpver);

    free(json_buf);

    if (strlen(config.name) == 0 || strlen(config.domain) == 0) {
        printf("Error: Missing required fields in JSON payload\n");
        return 1;
    }

    return apply_nginx_config(&config);
}
