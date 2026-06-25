#!/bin/bash

green='\033[0;32m'
red='\033[0;31m'
yellow='\033[0;33m'
blue='\033[0;34m'
purple='\033[0;35m'
cyan='\033[0;36m'
white='\033[0;37m'
bright_red='\033[1;31m'
bright_green='\033[1;32m'
bright_yellow='\033[1;33m'
bright_blue='\033[1;34m'
bright_purple='\033[1;35m'
bright_cyan='\033[1;36m'
bright_white='\033[1;37m'
nc='\033[0m' # No Color

echo -e "${green}------------------------------"
echo -e " Cosmic Fractal - Uninstaller"
echo -e "------------------------------${nc}"

appl=fractal-grow
binary=exfra
arch=$(uname -m)
os=$(uname -s)
version=0.4.0

# Verify 64bit architecture
if [[ "$arch" == "x86_64" || "$arch" == "amd64" ]]; then
    arch="64-bit"
# elif [[ "$arch" == "arm"* || "$arch" == "aarch64" || "$arch" == "arm64" ]]; then
#     arch="arm64"
else
    echo -e "${red}❌ Fail: ${yellow}Unsupported architecture: ${arch}${nc}"
    exit 1
fi

if [[ "$os" == "Linux" ]]; then
    os="linux"
# elif [[ "$os" == "Darwin" ]]; then
#     os="darwin"
else
    echo -e "${red}❌ Fail: ${yellow}Unsupported operating system${os}${nc}"
    exit 1
fi


# Use in  commands
# Local
PREFIX="$HOME/.local"
SUDO=""

$SUDO rm -f "$PREFIX/bin/${binary}"
# Uninstall also supporting files
if [ -n "$appl" ]; then
    # Safety check that appl is Non-empty 
    $SUDO rm -rf $PREFIX/share/${appl}/
else
    echo -e "${red}❌ Error in script: ${yellow}Not defined appl.${nc}"
    exit 1
fi

# Global
PREFIX="/usr/local"
SUDO="sudo"

$SUDO rm -f "$PREFIX/bin/${binary}"
# Uninstall also supporting files
if [ -n "$appl" ]; then
    # Safety check that appl is Non-empty 
    $SUDO rm -rf $PREFIX/share/${appl}/
else
    echo -e "${red}❌ Error in script: ${yellow}Not defined appl.${nc}"
    exit 1
fi

echo -e "${bright_green}Uninstall complete!${nc}"
