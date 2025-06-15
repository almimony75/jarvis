#!/bin/bash

# setup.sh
# ─────────────────────────────────────────────────────────────────────────────
# This script automates the installation and setup of all required dependencies
# for Jarvis, an offline voice assistant running on Linux.
#
# Components installed:
# • whisper.cpp — for real-time Speech-to-Text (STT)
# • llama.cpp — to run a local LLM (Language Model) server
# • Piper TTS — for high-quality offline Text-to-Speech
#
# What this script does:
# 1. Clones and builds the latest versions of each dependency from source.
# 2. Downloads pre-trained model files for Whisper, LLaMA, and Piper.
# 3. Copies compiled binaries into the local ./bin directory.
# 4. Moves required shared libraries (.so) into /usr/local/lib for system-wide linking.
# 5. Ensures /usr/local/lib is in the dynamic linker path via ldconfig.
#
# The final result is a fully functional setup ready to run with `./run.sh`.
# ─────────────────────────────────────────────────────────────────────────────


# --- Color Definitions ---
NC='\033[0m'         # No Color
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[1;34m'
CYAN='\033[1;36m'

info() { echo -e "${CYAN}[INFO] $1${NC}"; }
success() { echo -e "${GREEN}[✓] $1${NC}"; }
warn() { echo -e "${YELLOW}[!] $1${NC}"; }
error() { echo -e "${RED}[✗] $1${NC}"; }



center_text() {
  local term_width=$(tput cols)
  while IFS= read -r line; do
    local line_length=${#line}
    if (( line_length < term_width )); then
      local padding=$(( (term_width - line_length) / 2 ))
      printf "%*s%s\n" "$padding" "" "$line"
    else
      # If line longer than terminal width, just print it as-is
      echo "$line"
    fi
  done
}



echo -e "${BLUE}"
cat << "EOF" | center_text
███╗   ███╗██╗███╗   ███╗ ██████╗ ███╗   ██╗██╗   ██╗
████╗ ████║██║████╗ ████║██╔═══██╗████╗  ██║╚██╗ ██╔╝
██╔████╔██║██║██╔████╔██║██║   ██║██╔██╗ ██║ ╚████╔╝ 
██║╚██╔╝██║██║██║╚██╔╝██║██║   ██║██║╚██╗██║  ╚██╔╝  
██║ ╚═╝ ██║██║██║ ╚═╝ ██║╚██████╔╝██║ ╚████║   ██║   
╚═╝     ╚═╝╚═╝╚═╝     ╚═╝ ╚═════╝ ╚═╝  ╚═══╝   ╚═╝   
EOF
echo -e "${NC}"



set -e # Exit immediately if a command exits with a non-zero status

JARVIS_ROOT=$(pwd) 
LIB_DIR="/usr/local/lib"
BIN_DIR="$JARVIS_ROOT/bin"
MODELS_DIR="$JARVIS_ROOT/models"
TEMP_DIR="$JARVIS_ROOT/temp_deps" # Temporary directory for cloning repos

WHISPER_MODEL_URL="https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-base.en.bin"
LLAMA_MODEL_URL="https://huggingface.co/pek111/Meta-Llama-3.1-8B-Instruct-GGUF/resolve/main/Meta-Llama-3.1-8B-Instruct.Q4_0.gguf"
PIPER_MODEL_URL="https://huggingface.co/rhasspy/piper-voices/resolve/main/en/en_US/lessac/high/en_US-lessac-high.onnx"
PIPER_CONFIG_URL="https://huggingface.co/rhasspy/piper-voices/resolve/main/en/en_US/lessac/high/en_US-lessac-high.onnx.json"


# Create necessary directories if they don't exist
mkdir -p "$BIN_DIR"
mkdir -p "$MODELS_DIR"
mkdir -p "$TEMP_DIR"


# Detect distro and install packages
install_dependencies() {
    if command -v pacman &>/dev/null; then
        info "Detected Arch Linux"
        sudo pacman -S --noconfirm git gcc make curl wget portaudio alsa-utils cmake ffmpeg espeak-ng || {
            error "Pacman failed to install packages"; exit 1; }
    
    elif command -v apt &>/dev/null; then
        info "Detected Debian/Ubuntu"
        sudo apt update && sudo apt install -y git g++ make curl wget portaudio19-dev alsa-utils cmake ffmpeg espeak-ng || {
            error "APT failed to install packages"; exit 1; }
    
    elif command -v dnf &>/dev/null; then
        info "Detected Fedora"
        sudo dnf install -y git gcc-c++ make curl wget portaudio-devel alsa-utils cmake ffmpeg espeak-ng || {
            error "DNF failed to install packages"; exit 1; }

    elif command -v zypper &>/dev/null; then
        info "Detected openSUSE"
        sudo zypper install -y git gcc-c++ make curl wget portaudio-devel alsa-utils cmake ffmpeg espeak-ng || {
            error "Zypper failed to install packages"; exit 1; }

    else
        error "Unsupported or unknown package manager."
        exit 1
    fi

    success "All dependencies installed."
}

install_dependencies


# --- Helper function for downloading ---
download_file() {
    local url="$1"
    local output="$2"
    info "Downloading $url to $output..."
    if command -v wget >/dev/null 2>&1; then
        wget -q --show-progress -O "$output" "$url" || { error "wget download failed for $url"; return 1; }
    else
        curl -L -sS -o "$output" "$url" || { error "curl download failed for $url"; return 1; }
    fi
    success "Download complete: $output"
    return 0
}

echo ""
info "--- Setting up whisper.cpp ---"
cd "$TEMP_DIR"
if [ -d "whisper.cpp" ]; then
    warn "whisper.cpp directory already exists. Skipping clone."
else
    info "Cloning whisper.cpp..."
    git clone https://github.com/ggerganov/whisper.cpp.git
fi
cd whisper.cpp
info "Building whisper-cli..."
cmake -B build
cmake --build build --config Release -- -j$(nproc)

cp "./build/src/libwhisper.so.1" "$LIB_DIR"
cp "./build/ggml/src/libggml.so" "$LIB_DIR"
cp "./build/ggml/src/libggml-cpu.so" "$LIB_DIR"
cp "./build/ggml/src/libggml-base.so" "$LIB_DIR"
cp "./build/bin/whisper-cli" "$BIN_DIR/whisper-cli"
success "whisper-cli binary copied to $BIN_DIR"


echo ""
info "--- Setting up llama.cpp server ---"
cd "$TEMP_DIR"
if [ -d "llama.cpp" ]; then
    warn "llama.cpp directory already exists. Skipping clone."
else
    info "Cloning llama.cpp..."
    git clone https://github.com/ggml-org/llama.cpp.git
fi

cd llama.cpp
info "Building llama.cpp server..."
cmake -B build
cmake --build build --config Release -t llama-server -- -j$(nproc)


cp "./build/bin/libmtmd.so" "$LIB_DIR"
cp "./build/bin/libllama.so" "$LIB_DIR"
cp "./build/bin/llama-server" "$BIN_DIR/llama-server"
success "llama-server binary copied to $BIN_DIR"


echo ""
info "--- Setting up Piper TTS ---"
cd "$TEMP_DIR"

if [ -d "piper" ]; then
    warn "piper directory already exists. Skipping clone."
else
    info "Cloning piper..."
    git clone https://github.com/rhasspy/piper.git
fi

cd piper
info "Building piper..."
mkdir build && cd build
cmake ..
make -j$(nproc)
make install

cp "pi/lib/libonnxruntime.so.1.14.1" "$LIB_DIR"
cp "pi/lib/libpiper_phonemize.so.1" "$LIB_DIR"
cp piper "$BIN_DIR"
success "piper binary copied to $BIN_DIR"



echo ""
info "Downloading Piper voice model and config..."
download_file "$PIPER_MODEL_URL" "$MODELS_DIR/en_US-lessac-high.onnx"
download_file "$PIPER_CONFIG_URL" "$MODELS_DIR/en_US-lessac-high.onnx.json"

echo ""
info "Downloading Whisper base.en model..."
download_file "$WHISPER_MODEL_URL" "$MODELS_DIR/ggml-base.en.bin"

echo ""
info "Downloading Llama 3.1 8B Instruct GGUF model (Q4_K_M quantization)..."
download_file "$LLAMA_MODEL_URL" "$MODELS_DIR/Meta-Llama-3.1-8B-Instruct.Q4_0.gguf"



# --- Ensure /usr/local/lib is in the linker config ---
if ! grep -qs '^/usr/local/lib$' /etc/ld.so.conf /etc/ld.so.conf.d/* 2>/dev/null; then
    info "Adding /usr/local/lib to linker config..."
    echo "/usr/local/lib" | tee /etc/ld.so.conf.d/piper.conf
    ldconfig
else
    info "/usr/local/lib already present in linker config."
    ldconfig
fi

echo ""
info "--- Cleaning up temporary files ---"
cd "$JARVIS_ROOT"
rm -rf "$TEMP_DIR"

chmod +x buildMe.sh
chmod +x testMe.sh

echo ""
success "🎉 All dependencies for Jarvis have been installed!"
success "➡️  You can now build Jarvis using: ./buildMe.sh"
warn "👋 If you like Jarvis, please consider giving it a star on GitHub: https://github.com/almimony/jarvis"

echo ""
warn "🧪 To test all the binaries and ensure everything works, run: ./testMe.sh"
echo ""
