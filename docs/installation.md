---
title: "Installation"
description: "Installation procedure"
---

# Installation 📦
## Dependencies 🧩
- **SFML** (2D graphics and more) — to be installed *manually* beforehand (see [sfml website](https://www.sfml-dev.org/))
- lyra (C++ arg parser) — embedded as a subproject (see [original lyra source](https://github.com/bfgroup/Lyra))
- **tomlplusplus** — embedded as a subproject (also available through package managers)

## Using Installer Script 📜
This is applicable only to Linux as it uses compiled binary.  
Install **sfml** and **tomlplusplus** before using for example package manager.  
Then fetch Installer:
```shell
curl -O https://raw.githubusercontent.com/aKermit21/fractal-grow/main/scripts/install.sh
```
Run Installer:
```shell
chmod +x install.sh
./install.sh
```

> **_NOTE:_** `install.sh` script can be found also in project itself.

## Installing via PKGBUILD (Arch Linux) 📦

This method is for **Arch Linux and Arch-based distributions** (CachyOS, EndeavourOS, etc.) using the pacman package manager.

### Fetch the PKGBUILD
```shell
curl -O https://raw.githubusercontent.com/aKermit21/fractal-grow/main/scripts/PKGBUILD
```

### Build and install
```shell
makepkg -si
```
This builds the package and installs it with all required dependencies.

## By Project Compilation 🔨
### Get the Project
Clone the GitHub project:
```shell
git clone https://github.com/aKermit21/fractal-grow.git
```

### Compilation 🔨
The procedure below was tested on Linux (but shall work with possible adaptations also on MacOS and Windows).  
It is recommended to use the Meson build system, as it verifies dependencies, handles subprojects, enables automatic configuration, and supports explicit installation.

```shell
cd fractal-grow
mkdir build-release/
meson setup build-release/
cd build-release/
meson compile
./exfra [-h]   # to run the app directly from the build directory
```

### Optional: Explicit Installation of Program, Dependent Libraries, and Files in the System 
```shell
meson configure --prefix=$HOME/.local   # optionally, for Linux LOCAL installation
meson install
```
Manually copy the other images (`../image/*.jpg`) to the same location where the `Galaxy.jpg` file was copied.

To install the toml++ shared library in the system (which will NOT be installed automatically as a subproject):

#### Use a Package Manager
For example, with pacman:
```shell
sudo pacman -S tomlplusplus
```

#### Perform a Custom Build
One can also build and install it manually:

```shell
cd ../subprojects/tomlplusplus-3.4.0
mkdir build-lib/
meson setup build-lib/
cd build-lib/
meson install
[sudo ldconfig]   # update library cache
exfra [-h]     # now shall run from any location
```
Note that adding a custom library path may then be needed.

