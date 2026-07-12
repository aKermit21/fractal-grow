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

echo -e "${bright_blue}-----------------------------"
echo -e " Cosmic Fractal - Installer"
echo -e "-----------------------------${nc}"

temp_dir=$(mktemp -d)
if [ $? -ne 0 ]; then
    echo -e "${red}❌ Fail install application: ${yellow}Unable to create temporary directory${nc}"
    exit 1
fi

# Find latest version
if response=$(curl -s --max-time 5 "https://api.github.com/repos/aKermit21/fractal-grow/releases/latest"); then
    version=$(echo "$response" | grep '"tag_name"' | cut -d'"' -f4 )
    if [ -n "$version" ]; then
        # echo "$response"
        echo "latest version: ${version}"
    else
        echo -e "${red}❌ Failed to parse version from GitHub API:${nc}" >&2
        echo "$response"
        exit 1
    fi
else
    echo -e "${red}❌ Failed to fetch latest version from GitHub API${nc}" >&2
    echo "$response"
    exit 1
fi
  
appl=fractal-grow
binary=exfra
arch=$(uname -m)
os=$(uname -s)

cd "${temp_dir}"

# Verify 64bit architecture
if [[ "$arch" == "x86_64" || "$arch" == "amd64" ]]; then
    arch="64-bit"
# elif [[ "$arch" == "arm"* || "$arch" == "aarch64" || "$arch" == "arm64" ]]; then
#     arch="arm64"
else
    echo -e "${red}❌ Fail install ${appl}: ${yellow}Unsupported architecture: ${arch}${nc}"
    exit 1
fi

if [[ "$os" == "Linux" ]]; then
    os="linux"
# elif [[ "$os" == "Darwin" ]]; then
#     os="darwin"
else
    echo -e "${red}❌ Fail install ${appl}: ${yellow}Unsupported operating system${os}${nc}"
    exit 1
fi

file_name=${binary}_${version}_${os}_${arch}

# My URL for binaries (v0.5.0):
# https://github.com/aKermit21/fractal-grow/releases/download/v0.5.0/exfra_v0.5.0_linux_64-bit.tar.gz 

url="https://github.com/aKermit21/fractal-grow/releases/download/${version}/${file_name}.tar.gz"

echo -e "${bright_yellow}Downloading ${cyan}${appl} ${version} for ${os} ...${nc}"
curl -sLO "$url"

echo -e "${bright_yellow}Extracting ${cyan}${appl} files...${nc}"
tar -xzf "${file_name}.tar.gz"

# Choose type of installation

echo "Installation type:"
echo -e "  [U] per User (Local)   - Install to ${green}\$HOME/.local/${nc} (no root needed)"
echo -e "  [G] Global - Install to ${yellow}/usr/local/${nc} (requires sudo)"

while true; do
    read -p "Choose installation type [U/g]: (deafult: U) " choice
    choice=${choice:-U}  # Default to U
    
    case "$choice" in
        [Uu]*)
            PREFIX="$HOME/.local"
            SUDO=""
            INSTALL_TYPE="local"
            echo -e "${bright_yellow}Installing ${cyan}${appl} per User...${nc}"
            break
            ;;
        [Gg]*)
            PREFIX="/usr/local"
            SUDO="sudo"
            INSTALL_TYPE="global"
            # Check if user has sudo access
            if ! sudo -v 2>/dev/null; then
                echo "Error: You need sudo privileges for global installation"
                continue
            fi
            echo -e "${bright_yellow}Installing ${cyan}${appl} Globally...${nc}"
            break
            ;;
        *)
            echo -e "${red}Invalid choice. Please enter U or G.${nc}"
            ;;
    esac
done

echo Using PREFIX=$PREFIX

# Use in install commands
# Install exec binary
$SUDO install -Dm755 ${binary} "$PREFIX/bin/${binary}"
# Install also supporting files
$SUDO mkdir -p $PREFIX/share/${appl}/
$SUDO install -Dm644  text_fonts.ttf ${appl}-cfg.toml Galaxy.jpg Moon.jpg Earth.jpg LICENSE-fonts.txt "$PREFIX/share/${appl}/"
$SUDO mkdir -p $PREFIX/share/licenses/${appl}/
$SUDO install -Dm644  LICENSE LICENSE-fonts.txt  "$PREFIX/share/licenses/${appl}/"


if [[ $INSTALL_TYPE == local ]]; then
    if ! [[ ":$PATH:" == *":$HOME/.local/bin:"* ]]; then
      echo -e "${red}Path NOT Found. Please add ${white}\"${bright_cyan}\${HOME}/.local/bin${white}\" ${red}to PATH in your shell's config file.${nc}"

      echo 'for bash, add:'
      echo -e "${yellow}   export PATH=\"\${HOME}/.local/bin\":\${PATH}${nc} to ~/.bashrc"
      echo -e "${white}for zsh, add:"
      echo -e "${yellow}   export PATH=\"\${HOME}/.local/bin\":\${PATH}${nc} to ~/.zshrc"
      echo -e "${white}for fish, add:"
      echo -e "${yellow}   fish_add_path \"\$HOME/.local/bin\"${nc} to ~/.config/fish/config.fish"
      echo -e "${white}for ksh, add:"
      echo -e "${yellow}   export PATH=\"\${HOME}/.local/bin\":\${PATH}${nc} to ~/.kshrc"
      echo -e "${white}for csh, add:"
      echo -e "${yellow}   setenv PATH \"\${HOME}/.local/bin\":\${PATH}${nc} to ~/.cshrc"

      echo -e "${nc}Then please source your config file/relogin.${nc}"
    fi
fi

echo -e "🎉 ${bright_green}Installation complete!${nc}"
echo -e "${bright_cyan}You can type ${white}\"${bright_yellow}exfra${white}\" ${bright_cyan}to start!${nc}"

rm -rf "$temp_dir"
