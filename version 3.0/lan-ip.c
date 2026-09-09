#include "config.h"
#include <stdio.h>
#include <string.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void find_ip(void)
{
  snprintf(lan_ip, sizeof(lan_ip), "127.0.0.1");
  struct ifaddrs *ifaddr, *ifa;
  if (getifaddrs(&ifaddr) == 0)
  {
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
      if (ifa->ifa_addr == NULL || ifa->ifa_addr->sa_family != AF_INET) continue;

      if (strcmp(ifa->ifa_name, "lo") == 0 || strncmp(ifa->ifa_name, "wg", 2) == 0 || strncmp(ifa->ifa_name, "tailscale", 9) == 0 || strncmp(ifa->ifa_name, "docker", 6) == 0 || strncmp(ifa->ifa_name, "veth", 4) == 0) continue;

      struct sockaddr_in *pAddr = (struct sockaddr_in *)ifa->ifa_addr;
      inet_ntop(AF_INET, &pAddr->sin_addr, lan_ip, sizeof(lan_ip));
      break;
    }
    freeifaddrs(ifaddr);
  }
}