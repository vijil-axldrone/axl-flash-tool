#!/usr/bin/env bash
# win-deps-fetch.sh
# Downloads and extracts MSYS2 MinGW-w64 packages for Windows cross-compilation.
# Requirements: curl, 7z (p7zip-full), zstd or 7z with zst support
set -e

DEST="$(cd "$(dirname "$0")" && pwd)/win-deps"
MSYS2_MIRROR="https://mirror.msys2.org/mingw/mingw64"
TMP="$DEST/_tmp_extract"

mkdir -p "$DEST/include" "$DEST/lib"

# ─── Helper: get latest package filename from MSYS2 repo listing ────────────
latest_pkg() {
    local prefix="$1"
    curl -sfL "https://repo.msys2.org/mingw/mingw64/" \
        | grep -oP "href=\"${prefix}[^\"]+\\.zst\"" \
        | grep -v '\.sig' \
        | grep -oP '(?<=href=")[^"]+' \
        | tail -1
}

# ─── Helper: download + extract one package ──────────────────────────────────
fetch_pkg() {
    local name="$1"
    local filename="$2"

    echo ""
    echo "=== $name ==="

    if [ -z "$filename" ]; then
        echo "  ERROR: Could not find package on MSYS2 mirror. Skipping."
        return 0
    fi

    local url="$MSYS2_MIRROR/$filename"
    local outfile="$DEST/$filename"

    if [ -f "$outfile" ]; then
        echo "  Already downloaded: $filename"
    else
        echo "  Downloading from: $url"
        curl -fL --progress-bar -o "$outfile" "$url"
    fi

    echo "  Extracting..."
    rm -rf "$TMP" && mkdir -p "$TMP"

    # .pkg.tar.zst: decompress with zstd if available, else use 7z
    if command -v zstd &>/dev/null; then
        zstd -d "$outfile" -o "$TMP/pkg.tar" -f -q
        tar -xf "$TMP/pkg.tar" -C "$TMP"
    else
        # 7z can handle .zst on newer versions; fallback two-step via 7z
        7z x -so "$outfile" 2>/dev/null | tar -x -C "$TMP" 2>/dev/null || \
        (7z x "$outfile" -o"$TMP" -y > /dev/null && 7z x "$TMP"/*.tar -o"$TMP" -y > /dev/null)
    fi

    # Copy headers
    if [ -d "$TMP/mingw64/include" ]; then
        cp -rn "$TMP/mingw64/include/." "$DEST/include/" 2>/dev/null || \
        cp -r  "$TMP/mingw64/include/." "$DEST/include/"
        echo "  Headers installed."
    fi

    # Copy static libs
    if [ -d "$TMP/mingw64/lib" ]; then
        find "$TMP/mingw64/lib" -maxdepth 1 -name "*.a" -exec cp -n {} "$DEST/lib/" \; 2>/dev/null || \
        find "$TMP/mingw64/lib" -maxdepth 1 -name "*.a" -exec cp    {} "$DEST/lib/" \;
        echo "  Static libs installed."
    fi

    rm -rf "$TMP"
    echo "  Done."
}

# ─── Check for zstd (preferred) or 7z ────────────────────────────────────────
if ! command -v zstd &>/dev/null && ! command -v 7z &>/dev/null; then
    echo "ERROR: Please install zstd or p7zip-full first:"
    echo "  sudo apt install zstd"
    exit 1
fi

# Install zstd if missing (needed to decompress .zst packages properly)
if ! command -v zstd &>/dev/null; then
    echo "Installing zstd for .zst decompression..."
    sudo apt-get install -y zstd -q
fi

echo "Querying MSYS2 mirror for latest package versions..."

OPENSSL_PKG=$(latest_pkg "mingw-w64-x86_64-openssl-")
CURL_PKG=$(latest_pkg "mingw-w64-x86_64-curl-winssl-")      # winssl variant needs no openssl.dll
ZLIB_PKG=$(latest_pkg "mingw-w64-x86_64-zlib-")
SERIALPORT_PKG=$(latest_pkg "mingw-w64-x86_64-libserialport-")

echo "  openssl:       $OPENSSL_PKG"
echo "  curl (winssl): $CURL_PKG"
echo "  zlib:          $ZLIB_PKG"
echo "  libserialport: $SERIALPORT_PKG"

fetch_pkg "OpenSSL"        "$OPENSSL_PKG"
fetch_pkg "curl (WinSSL)"  "$CURL_PKG"
fetch_pkg "zlib"           "$ZLIB_PKG"
fetch_pkg "libserialport"  "$SERIALPORT_PKG"

echo ""
echo "========================================"
echo "All Windows dependencies installed into: $DEST"
echo ""
echo "Headers available:"
ls "$DEST/include" | sed 's/^/  /'
echo ""
echo "Static libs available:"
ls "$DEST/lib" | grep '\.a$' | sed 's/^/  /'
echo ""
echo "Now run:  make windows"
echo "========================================"
