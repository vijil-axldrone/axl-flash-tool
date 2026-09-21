CC_LINUX  = gcc
CC_WIN    = x86_64-w64-mingw32-gcc

CFLAGS    = -Wall -Iinclude -Isrc -Isrc/requests

# Auto-finds all .c files recursively in src/
SRCS = main.c $(shell find src -name "*.c")

# ── Linux ─────────────────────────────────────────────────────────────────────
LIBS_LINUX = -lserialport -lssl -lcrypto -ljson-c -lcjson -lcurl

# ── Windows cross-compile ─────────────────────────────────────────────────────
# Windows-compatible (MinGW) headers and static libs go in win-deps/
# Run:  make win-deps-setup   to download and build them automatically.
WIN_DEPS  = win-deps
WIN_CFLAGS = $(CFLAGS) -I$(WIN_DEPS)/include -DCURL_STATICLIB
WIN_LDFLAGS = -L$(WIN_DEPS)/lib
WIN_LIBS  = -static -lserialport -lssl -lcrypto -ljson-c -lcjson -lcurl \
            -lws2_32 -lcrypt32 -lsecur32 -lbcrypt -lz -lsetupapi -lcfgmgr32

# Default: build both
all: axlflash axlflash.exe

# ── Linux target ─────────────────────────────────────────────────────────────
axlflash: $(SRCS)
	$(CC_LINUX) $(CFLAGS) $(SRCS) -o $@ $(LIBS_LINUX)

# ── Windows cross-compile target ─────────────────────────────────────────────
axlflash.exe: $(SRCS) | win-deps-check
	$(CC_WIN) $(WIN_CFLAGS) $(SRCS) -o $@ $(WIN_LDFLAGS) $(WIN_LIBS)

# ── Check that win-deps exist before trying to build ─────────────────────────
win-deps-check:
	@if [ ! -d "$(WIN_DEPS)/include/cjson" ] || [ ! -d "$(WIN_DEPS)/include/openssl" ]; then \
	    echo ""; \
	    echo "ERROR: Windows cross-compilation dependencies are missing."; \
	    echo "Run:  make win-deps-setup  to download and build them."; \
	    echo ""; \
	    exit 1; \
	fi

# ── Download and cross-compile Windows dependencies ───────────────────────────
win-deps-setup:
	@echo "=== Setting up Windows cross-compilation dependencies ==="
	@mkdir -p $(WIN_DEPS)/src $(WIN_DEPS)/include $(WIN_DEPS)/lib

	@echo "--- cJSON ---"
	@cd $(WIN_DEPS)/src && \
	  [ -d cJSON ] || git clone --depth=1 https://github.com/DaveGamble/cJSON.git && \
	  cd cJSON && \
	  x86_64-w64-mingw32-gcc -O2 -c cJSON.c -o cJSON.o && \
	  x86_64-w64-mingw32-ar rcs ../../lib/libcjson.a cJSON.o && \
	  mkdir -p ../../include/cjson && cp cJSON.h ../../include/cjson/
	@echo "cJSON done."

	@echo "--- json-c ---"
	@cd $(WIN_DEPS)/src && \
	  [ -d json-c ] || git clone --depth=1 https://github.com/json-c/json-c.git && \
	  cd json-c && mkdir -p build-win && cd build-win && \
	  cmake .. -DCMAKE_SYSTEM_NAME=Windows \
	           -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
	           -DCMAKE_INSTALL_PREFIX=../../../ \
	           -DBUILD_SHARED_LIBS=OFF > /dev/null && \
	  make -j$(shell nproc) install > /dev/null
	@echo "json-c done."

	@echo ""
	@echo "NOTE: OpenSSL and libcurl for MinGW require more steps."
	@echo "The easiest way is to use MXE (M cross environment):"
	@echo "  https://mxe.cc/#tutorial"
	@echo ""
	@echo "Or grab pre-built MinGW packages from:"
	@echo "  https://packages.msys2.org/search?q=mingw-w64-x86_64-openssl"
	@echo "  https://packages.msys2.org/search?q=mingw-w64-x86_64-curl"
	@echo "  https://packages.msys2.org/search?q=mingw-w64-x86_64-libserialport"
	@echo ""
	@echo "Extract them and copy include/ and lib/ into $(WIN_DEPS)/"

# ── Convenience aliases ───────────────────────────────────────────────────────
linux: axlflash
windows: axlflash.exe

# ── Clean ─────────────────────────────────────────────────────────────────────
clean:
	rm -f axlflash axlflash.exe $(shell find src -name "*.o") main.o

clean-win-deps:
	rm -rf $(WIN_DEPS)

# ── Run (Linux only) ──────────────────────────────────────────────────────────
run: axlflash
	./axlflash
