cmake_minimum_required(VERSION 3.10)

# Define project name and language
project(NAP C)

# Set the C standard (C11 is standard, adjust if needed)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Option A: Option 1 approach (Single executable compiled from multiple sources)
add_executable(nap start.c update.c lan-ip.c pingora/pingora.c nginx/nginx.c nginx/nginx_json.c nginx/domain.c nginx/presets.c nginx/gui-install.c nginx/jellyfin.c nginx/plex.c nginx/nextcloud.c nginx/gui.c nginx/manual.c nginx/proxy.c nginx/directory.c nginx/certbot.c)

# Option B: Option 2 approach (If you want two separate executables)
# add_executable(start start.c)
# add_executable(presets presets.c)