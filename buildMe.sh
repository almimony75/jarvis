#!/bin/bash

# --- Color Definitions ---
NC='\033[0m'         # No Color
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[1;34m'
CYAN='\033[1;36m'

info()    { echo -e "${CYAN}[INFO] $1${NC}"; }
success() { echo -e "${GREEN}[✓] $1${NC}"; }
warn()    { echo -e "${YELLOW}[!] $1${NC}"; }
error()   { echo -e "${RED}[✗] $1${NC}"; }

LOGFILE="build.log"

clean_build() {
    if [ -f bin/jarvis ]; then
        info "Cleaning old build..."
        rm -f bin/jarvis
        success "Old build removed."
    else
        warn "No previous build found to clean."
    fi
}

print_usage() {
    echo "Usage: $0 [clean]"
    echo "  clean  - Remove old build before compiling"
}

# Exit on error and print error message
trap 'error "Build failed at line $LINENO"; exit 1' ERR
set -e

# Parse optional argument
if [[ "$1" == "clean" ]]; then
    clean_build
elif [[ -n "$1" ]]; then
    print_usage
    exit 1
fi


info "Starting build at $(date)"
info "Compiling sources..."

# Redirect output and errors to logfile, but show errors on screen too
if g++ src/main.cpp src/recorder.cpp src/stt.cpp src/tts.cpp src/brain.cpp -o bin/jarvis -I include -lcurl -lportaudio > >(tee "$LOGFILE") 2> >(tee -a "$LOGFILE" >&2); then
    info "Build finished at $(date)"
    success "Jarvis built successfully! Binary is at bin/jarvis"
else
    error "Compilation failed. Check $LOGFILE for details."
    info "Build finished at $(date)"
    exit 1
fi


