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
if response=$(curl -s --max-time 5 "https://api.github.com/repos/aKermit21/Cosmic-fractal/releases/latest"); then
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
  
# Long application name
appl=Cosmic-fractal
# Binary, package, short application name age
binary=cosfra
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

# My example Github URL for binaries (v0.5.0):
# https://github.com/aKermit21/Cosmic-fractal/releases/download/v0.8.0/cosfra_v0.8.0_linux_64-bit.tar.gz 
# My project URL for additional background images, ex:
# https://pcc21.com/upload/store/ny_city.jpg

url="https://github.com/aKermit21/Cosmic-fractal/releases/download/${version}/${file_name}.tar.gz"
url_store="https://pcc21.com/upload/store"

images_from_store=("burj_khalifa.jpg" "ny_city.jpg")

echo -e "${bright_yellow}Downloading ${cyan}${appl} ${version} for ${os} ...${nc}"
curl -sLO --max-time 5 "$url"

# Verify file exists and its real - not just error message (too short)
if [ ! -f "${file_name}.tar.gz" ]; then
    echo -e "${red}❌ Failed to download file ${file_name}.tar.gz !${nc}"
    exit 1
elif [ "$(wc -c < "${file_name}.tar.gz")" -lt 100 ]; then
    echo -e "${red}❌ Looks downloaded file ${file_name}.tar.gz contains Error message:${nc}"
    cat "${file_name}.tar.gz"
    exit 1
fi

echo -e "${bright_yellow}Extracting ${cyan}${appl} files...${nc}"
if tar -xzf "${file_name}.tar.gz"; then
    echo -e "${green}   Extracted.${nc}"
else
    echo -e "${red}❌ Failed to extract file ${file_name}.tar.gz !${nc}"
    exit 1
fi

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

# Install exec binary
$SUDO install -Dm755 ${binary} "$PREFIX/bin/${binary}"

# Install basic supporting files
$SUDO mkdir -p $PREFIX/share/${binary}/
# all from GitHub page
$SUDO install -Dm644  text_fonts.ttf ${binary}-cfg.toml Galaxy.jpg Moon.jpg Earth.jpg LICENSE-fonts.txt "$PREFIX/share/${binary}/"
$SUDO mkdir -p $PREFIX/share/licenses/${binary}/
$SUDO install -Dm644  LICENSE LICENSE-fonts.txt  "$PREFIX/share/licenses/${binary}/"

# Additional Pictures fetched directly from dedicated project/store url
echo -e "${bright_yellow}Fetching ${cyan}additional ${appl} pictures...${nc}"

#Additional Pictures placement
for image in "${images_from_store[@]}"; do
    echo "${bright_blue}  $image${nc}$"
    license=LICENSE_"${image%.jpg}.txt"
    echo "${blue}  $license"
    curl -sLO "$url_store/$image"
    curl -sLO "$url_store/$license"
    if [ $? -eq 0 ]; then
        $SUDO install -Dm644  $image $license "$PREFIX/share/${binary}/"
        $SUDO install -Dm644  $license  "$PREFIX/share/licenses/${binary}/"
    else
        echo "${red}Failed: curl returned error code while loading additional picture $? ${nc}"
    fi
done


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
echo -e "${bright_cyan}You can type ${white}\"${bright_yellow}cosfra${white}\" ${bright_cyan}to start!${nc}"

rm -rf "$temp_dir"
