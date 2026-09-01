#!/usr/bin/env bash
set -e

TEMP_DEB="$(mktemp /tmp/auto-config-XXXXXX.deb)"
URL="https://auto-config.sizablesplash.com/older-releases/auto-config_1.1.2_amd64.deb"

echo "Downloading NGINX Auto Program..."
curl -sSL "$URL" -o "$TEMP_DEB"

echo "Installing package..."
sudo apt-get update -qq
sudo apt-get install -y "$TEMP_DEB"

echo "Cleaning up..."
rm -f "$TEMP_DEB"

echo "Installation complete!"
