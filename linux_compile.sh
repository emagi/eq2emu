#!/bin/bash

LOG_FILE="$(pwd)/eq2emu_install.log"
EQ2EMU_HOME_DIR="/home/eq2emu"
PREMAKE5_PKG="https://github.com/premake/premake-core/releases/download/v5.0.0-beta2/premake-5.0.0-beta2-linux.tar.gz"
PREMAKE5_FILE="premake-5.0.0-beta2-linux.tar.gz"
RECAST_GIT="https://github.com/recastnavigation/recastnavigation.git"
FMT_GIT="https://github.com/fmtlib/fmt.git"
EQ2SOURCE_GIT="https://github.com/emagi/eq2emu.git"
EQ2CONTENT_GIT="https://github.com/emagi/eq2emu-content.git"
EQ2MAPS_GIT="https://github.com/emagi/eq2emu-maps.git"

log() {
    echo "$(date +"%Y-%m-%d %H:%M:%S") - $1" | tee -a "$LOG_FILE"
}

handle_error() {
    log "ERROR: $1"
    exit 1
}

install_packages() {
    if command -v apt-get >/dev/null; then
        log "Detected Debian-based system. Using apt-get to install packages."
        sudo apt-get update
        sudo apt-get install -y git make mariadb-client automake g++ libsdl2-dev libmariadb-dev libboost-all-dev libreadline-dev cmake liblua5.4-dev lua5.4 libcrypto++-dev wget libglfw3-dev libgl1-mesa-dev libglu1-mesa-dev libglm-dev || handle_error "Failed to install packages using apt-get."
    elif command -v dnf >/dev/null; then
        log "Detected RHEL/Fedora-based system. Using dnf to install packages."
        sudo dnf install -y git make mariadb automake gcc-c++ SDL2-devel mariadb-devel boost-devel readline-devel cmake lua-devel lua crypto++-devel wget glfw-devel mesa-libGL-devel mesa-libGLU-devel glm-devel || handle_error "Failed to install packages using dnf."
    elif command -v zypper >/dev/null; then
        log "Detected SUSE-based system. Using zypper to install packages."
        sudo zypper install -y git make mariadb automake gcc-c++ libSDL2-devel libmariadb-devel boost-devel readline-devel cmake liblua5_4-devel lua5_4 libcrypto++-devel wget libglfw3-devel Mesa-libGL-devel Mesa-libGLU-devel glm-devel || handle_error "Failed to install packages using zypper."
    else
        handle_error "Unsupported Linux distribution."
    fi
}


log "Starting EQ2EMU installation..."

# Install necessary packages
install_packages

# Create the home directory
log "Creating home directory..."
mkdir -p "${EQ2EMU_HOME_DIR}" || handle_error "Failed to create home directory."
cd "${EQ2EMU_HOME_DIR}" || handle_error "Failed to navigate to home directory."

# Download and install premake5
log "Downloading premake5..."
wget ${PREMAKE5_PKG} || handle_error "Failed to download premake5."
log "Extracting premake5..."
tar -xzvf ${PREMAKE5_FILE} || handle_error "Failed to extract premake5."

# Clone Recast repository
log "Cloning Recast repository..."
git clone ${RECAST_GIT} || handle_error "Failed to clone Recast repository."
cp premake5 recastnavigation/RecastDemo || handle_error "Failed to copy premake5 to RecastDemo."

# Compile Recast
log "Compiling Recast..."
cd recastnavigation/RecastDemo || handle_error "Failed to navigate to RecastDemo."
./premake5 gmake2 || handle_error "Failed to generate makefiles with premake5."
cd Build/gmake2 || handle_error "Failed to navigate to Build/gmake2."
make || handle_error "Failed to compile Recast."

# Clone and compile EQ2EMU source
log "Cloning and compiling EQ2EMU source..."
cd ${EQ2EMU_HOME_DIR} || handle_error "Failed to navigate to home directory."
git clone ${FMT_GIT} || handle_error "Failed to clone fmt repository."
git clone ${EQ2SOURCE_GIT} || handle_error "Failed to clone EQ2 source repository."

cd ${EQ2EMU_HOME_DIR}/eq2emu/source/LoginServer || handle_error "Failed to navigate to LoginServer directory."
git pull || handle_error "Failed to pull latest changes for LoginServer."
make clean || handle_error "Failed to clean LoginServer build."
make -j$(nproc) || handle_error "Failed to compile LoginServer."
cp login ${EQ2EMU_HOME_DIR}/eq2emu/server/ || handle_error "Failed to copy LoginServer executable."

cd ${EQ2EMU_HOME_DIR}/eq2emu/source/WorldServer || handle_error "Failed to navigate to WorldServer directory."
sed -i "/C_Flags/ s|-I/eq2emu/|-I${EQ2EMU_HOME_DIR}/|g" makefile || handle_error "Failed to update makefile C_Flags"
sed -i "/LD_Flags/ s|-L/eq2emu/|-L${EQ2EMU_HOME_DIR}/|g" makefile || handle_error "Failed to update makefile LD_Flags"
git pull || handle_error "Failed to pull latest changes for WorldServer."
make clean || handle_error "Failed to clean WorldServer build."
make -j$(nproc) || handle_error "Failed to compile WorldServer."
cp eq2world ${EQ2EMU_HOME_DIR}/eq2emu/server/ || handle_error "Failed to copy WorldServer executable."

# Clone EQ2 content and maps
log "Cloning EQ2 content and maps..."
cd ${EQ2EMU_HOME_DIR} || handle_error "Failed to navigate to home directory."
git clone ${EQ2CONTENT_GIT} || handle_error "Failed to clone EQ2 content repository."
git clone ${EQ2MAPS_GIT} || handle_error "Failed to clone EQ2 maps repository."

# Copy content and maps to server
log "Copying content and maps to server..."
cd ${EQ2EMU_HOME_DIR}/eq2emu-content || handle_error "Failed to navigate to eq2emu-content directory."
cp -r ItemScripts Quests RegionScripts SpawnScripts Spells ZoneScripts PlayerScripts ${EQ2EMU_HOME_DIR}/eq2emu/server/ || handle_error "Failed to copy content scripts."

cd ${EQ2EMU_HOME_DIR}/eq2emu-maps || handle_error "Failed to navigate to eq2emu-maps directory."
cp -r Maps Regions ${EQ2EMU_HOME_DIR}/eq2emu/server/ || handle_error "Failed to copy maps."

log "EQ2EMU installation completed successfully."
