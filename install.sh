#!/usr/bin/env bash
set -e

TEMP_DEB="$(mktemp /tmp/auto-config-XXXXXX.deb)"
URL="https://repo.sizablesplash.com/auto-config/releases/auto-config_2.0.0_amd64.deb"

echo "Downloading NGINX Auto Program... making sure I see this"
curl -sSL "$URL" -o "$TEMP_DEB"

echo "Installing package..."
sudo apt-get update -qq
sudo apt-get install -y "$TEMP_DEB"

echo "Cleaning up..."
rm -f "$TEMP_DEB"

echo "Installation complete!"
