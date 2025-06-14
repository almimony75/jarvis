#!/bin/bash


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

all_ok=true

echo ""
info "Testing whisper-cli..."
if ./bin/whisper-cli --help > /dev/null 2>&1; then
    success "✅ whisper-cli is working"
else
    error "❌ whisper-cli failed"
    all_ok=false
fi

info "Testing llama-server..."
if ./bin/llama-server --help > /dev/null 2>&1; then
    success "✅ llama-server is working"
else
    error "❌ llama-server failed"
    all_ok=false
fi

info "Testing piper..."
if ./bin/piper --help > /dev/null 2>&1; then
    success "✅ piper is working"
else
    error "❌ piper failed"
    all_ok=false
fi

echo ""
if [ "$all_ok" = true ]; then
    success "🎉 Everything is working great! Run build.sh"
    warn "👋 If you like Jarvis, please consider giving it a star on GitHub: https://github.com/almimony/jarvis"
else
    error "💥 One or more components failed. Please read above for Errors."
    exit 1
fi