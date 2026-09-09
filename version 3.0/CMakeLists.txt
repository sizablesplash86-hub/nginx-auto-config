cmake_minimum_required(VERSION 3.10)

# Define project name and language
project(NAP C)

# Set the C standard (C11 is standard, adjust if needed)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Option A: Option 1 approach (Single executable compiled from multiple sources)
add_executable(nap nginx_json.c start.c update.c lan-ip.c domain.c presets.c gui-install.c jellyfin.c plex.c nextcloud.c gui.c manual.c proxy.c directory.c certbot.c)

# Option B: Option 2 approach (If you want two separate executables)
# add_executable(start start.c)
# add_executable(presets presets.c)
