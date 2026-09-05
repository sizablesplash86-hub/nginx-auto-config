#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(void) {
    struct ifaddrs *ifaddr, *ifa;
    char ip[INET_ADDRSTRLEN];

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs failed");
        return EXIT_FAILURE;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL || ifa->ifa_addr->sa_family != AF_INET) {
            continue;
        }

        // Skip loopback, WireGuard, and Tailscale interfaces
        if (strcmp(ifa->ifa_name, "lo") == 0 ||
            strncmp(ifa->ifa_name, "wg", 2) == 0 ||
            strncmp(ifa->ifa_name, "tailscale", 9) == 0 ||
            strncmp(ifa->ifa_name, "docker", 6) == 0) {
            continue;
        }

        struct sockaddr_in *pAddr = (struct sockaddr_in *)ifa->ifa_addr;
        inet_ntop(AF_INET, &pAddr->sin_addr, ip, sizeof(ip));

        printf("Primary LAN Interface (%s): %s\n", ifa->ifa_name, ip);
    }

    freeifaddrs(ifaddr);
    return EXIT_SUCCESS;
}
