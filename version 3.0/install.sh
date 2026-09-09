#!/usr/bin/env bash
set -e

# Configuration
DEB_URL="https://repo.sizablesplash.com/auto-config/releases/v3.0.0/auto-config_3.0.0_amd64.deb"
WEB_URL_INDEX="https://repo.sizablesplash.com/auto-config/releases/gui/index.html"
WEB_URL_API="https://repo.sizablesplash.com/auto-config/releases/gui/api.php"

TEMP_DEB="$(mktemp /tmp/auto-config-XXXXXX.deb)"

# Ensure script is run with sudo privileges
if [ "$EUID" -ne 0 ]; then
    echo "Please run this script with sudo or as root."
    exit 1
fi

echo "=================================================="
echo "  NGINX Auto Program (N.A.P.) v3.0.0 Installer"
echo "=================================================="

# Download and install core binary package
echo "--> Downloading NGINX Auto Program binary..."
curl -sSL "$DEB_URL" -o "$TEMP_DEB"

echo "--> Installing package..."
apt-get update -qq
apt-get install -y "$TEMP_DEB"
rm -f "$TEMP_DEB"

# Prompt for Web GUI Installation
INSTALL_GUI="n"
if [ -t 0 ]; then
    read -rp "Would you like to install the Web GUI dashboard? [y/N]: " USER_INPUT
    case "$USER_INPUT" in
        [yY][eE][sS]|[yY])
            INSTALL_GUI="y"
            ;;
        *)
            INSTALL_GUI="n"
            ;;
    esac
fi

if [ "$INSTALL_GUI" = "y" ]; then
    echo "--> Installing Web GUI..."

    # Detect Web Server Root
    WEB_ROOT="/var/www/auto-config"
    if [ ! -d "$WEB_ROOT" ]; then
        mkdir -p "$WEB_ROOT"
    fi

    # Download GUI Interface Files
    echo "--> Fetching Web GUI interface components..."
    curl -sSL "$WEB_URL_INDEX" -o "$WEB_ROOT/index.html"
    curl -sSL "$WEB_URL_API" -o "$WEB_ROOT/api.php"

    # Set web server ownership (www-data)
    if id "www-data" &>/dev/null; then
        chown -R www-data:www-data "$WEB_ROOT"
        chmod 644 "$WEB_ROOT/index.html"
        chmod 644 "$WEB_ROOT/api.php"
    fi

    # Configure sudoers entry for non-interactive execution from api.php
    SUDOERS_FILE="/etc/sudoers.d/nginx-auto-gui"
    echo "--> Configuring passwordless sudo permissions for www-data..."
    echo "www-data ALL=(ALL) NOPASSWD: /usr/local/bin/nginx-auto --json *" > "$SUDOERS_FILE"
    chmod 0440 "$SUDOERS_FILE"

    echo "--> Web GUI installation complete! Files deployed to $WEB_ROOT."
fi

echo "=================================================="
echo "Installation completed successfully!"
echo "=================================================="