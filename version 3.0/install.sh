#!/usr/bin/env bash
set -e

# Configuration
DEB_URL="https://repo.sizablesplash.com/auto-config/releases/v3.0.0/auto-config_3.0.0_amd64.deb"
WEB_URL_INDEX="https://repo.sizablesplash.com/auto-config/releases/gui/gui.html"
WEB_URL_API="https://repo.sizablesplash.com/auto-config/releases/gui/gui.php"

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
    echo "--> Installing Web GUI dependencies..."

    # Install PHP and FastCGI
    apt-get install -y -qq php-fpm php-json > /dev/null

    # Detect the correct PHP-FPM socket location on Debian/Ubuntu
    PHP_SOCK=""
    if [ -S "/run/php/php-fpm.sock" ]; then
        PHP_SOCK="unix:/run/php/php-fpm.sock"
    else
        # Find active PHP version socket (e.g., /run/php/php8.2-fpm.sock)
        DETECTED_SOCK=$(ls /run/php/php*-fpm.sock 2>/dev/null | head -n 1)
        if [ -n "$DETECTED_SOCK" ]; then
            PHP_SOCK="unix:$DETECTED_SOCK"
        else
            PHP_SOCK="unix:/run/php/php-fpm.sock"
        fi
    fi

    # Web Directory Setup
    GUI_DIR="/var/www/auto-config"
    mkdir -p "$GUI_DIR"

    echo "--> Fetching Web GUI interface components..."
#   curl -sSL "$WEB_URL_INDEX" -o "$GUI_DIR/index.html"
#   curl -sSL "$WEB_URL_API" -o "$GUI_DIR/api.php"

    if id "www-data" &>/dev/null; then
        chown -R www-data:www-data "$GUI_DIR"
        chmod 644 "$GUI_DIR/index.html"
        chmod 644 "$GUI_DIR/api.php"
    fi

    # Configure sudoers entry for www-data execution
    SUDOERS_FILE="/etc/sudoers.d/nginx-auto-gui"
    echo "www-data ALL=(ALL) NOPASSWD: /usr/local/bin/nginx-auto --json *" > "$SUDOERS_FILE"
    chmod 0440 "$SUDOERS_FILE"

    # Deploy NGINX Configuration Site Block
    echo "--> Configuring NGINX site block for Web GUI (Port 3487)..."
    CONF_AVAILABLE="/etc/nginx/sites-available/auto-config-gui"
    CONF_ENABLED="/etc/nginx/sites-enabled/auto-config-gui"

    cat <<EOF > "$CONF_AVAILABLE"
server {
  listen 3487;

  server_name _;

  root /var/www/auto-config;
  index index.php index.html index.htm;

  location / {
    autoindex on;
    try_files \$uri \$uri/ =404;
  }

  location ~ \.php$ {
    include snippets/fastcgi-php.conf;
    fastcgi_pass $PHP_SOCK;
  }
}
EOF

    # Enable site and test configuration
    ln -sf "$CONF_AVAILABLE" "$CONF_ENABLED"

    echo "--> Verifying and reloading NGINX..."
    if nginx -t >/dev/null 2>&1; then
        systemctl reload nginx
        echo "--> Web GUI successfully configured on 127.0.0.1:3487"
    else
        echo "--> Warning: NGINX configuration test failed. Please check /etc/nginx/sites-available/auto-config-gui manually."
    fi
fi

echo "=================================================="
echo "Installation completed successfully! Run 'nginx-auto' to run the program"
echo "=================================================="