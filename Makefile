# Chess Game Makefile with Local Raylib and libjuice
# Cross-platform support for Linux and Windows (MinGW/MSYS2)

CC = gcc
AR = ar
CFLAGS = -Wall -Wextra -O2 -I./raylib/src -I./libjuice/include

TARGET = chess
SRCS = main.c board.c moves.c check.c ui.c menu.c history.c constants.c clock.c network.c multiplayer.c
OBJS = $(SRCS:.c=.o)
HEADERS = types.h board.h moves.h check.h ui.h menu.h history.h clock.h network.h multiplayer.h

RAYLIB_DIR = raylib
RAYLIB_LIB = $(RAYLIB_DIR)/src/libraylib.a

LIBJUICE_DIR = libjuice
LIBJUICE_LIB = $(LIBJUICE_DIR)/libjuice.a

# Platform detection
ifeq ($(OS),Windows_NT)
    PLATFORM = WINDOWS
    TARGET := $(TARGET).exe
    CFLAGS += -DJUICE_STATIC
    LDFLAGS = -lopengl32 -lgdi32 -lwinmm -lws2_32 -lbcrypt -static -lpthread
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        PLATFORM = LINUX
        LDFLAGS = -lGL -lm -lpthread -ldl -lrt -lX11
    else ifeq ($(UNAME_S),Darwin)
        PLATFORM = MACOS
        LDFLAGS = -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo -lpthread
    else
        ifneq (,$(findstring MINGW,$(UNAME_S)))
            PLATFORM = WINDOWS
            TARGET := $(TARGET).exe
            CFLAGS += -DJUICE_STATIC
            LDFLAGS = -lopengl32 -lgdi32 -lwinmm -lws2_32 -lbcrypt -static -lpthread
        else ifneq (,$(findstring MSYS,$(UNAME_S)))
            PLATFORM = WINDOWS
            TARGET := $(TARGET).exe
            CFLAGS += -DJUICE_STATIC
            LDFLAGS = -lopengl32 -lgdi32 -lwinmm -lws2_32 -lbcrypt -static -lpthread
        endif
    endif
endif

JUICE_CFLAGS = -O2 -fPIC -fvisibility=hidden -DJUICE_EXPORTS -DUSE_NETTLE=0 -I$(LIBJUICE_DIR)/include/juice

RM = rm -f
RMDIR = rm -rf

.PHONY: all clean raylib libjuice clean-raylib clean-libjuice clean-all help

all: $(TARGET)

$(TARGET): $(RAYLIB_LIB) $(LIBJUICE_LIB) $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(RAYLIB_LIB) $(LIBJUICE_LIB) $(LDFLAGS)

%.o: %.c $(HEADERS) $(LIBJUICE_LIB)
	$(CC) $(CFLAGS) -c $< -o $@

# Raylib
raylib: $(RAYLIB_LIB)

$(RAYLIB_LIB):
	@if [ ! -d "$(RAYLIB_DIR)" ]; then \
		echo "Downloading raylib..."; \
		git clone --depth 1 https://github.com/raysan5/raylib.git $(RAYLIB_DIR); \
	fi
	@if [ ! -f "$(RAYLIB_LIB)" ]; then \
		echo "Building raylib for $(PLATFORM)..."; \
		$(MAKE) -C $(RAYLIB_DIR)/src PLATFORM=PLATFORM_DESKTOP; \
	fi

# Libjuice - download and build in a single recipe
libjuice: $(LIBJUICE_LIB)

$(LIBJUICE_LIB):
	@if [ ! -d "$(LIBJUICE_DIR)" ]; then \
		echo "Downloading libjuice..."; \
		git clone --depth 1 https://github.com/paullouisageneau/libjuice.git $(LIBJUICE_DIR); \
	fi
	@if [ ! -f "$(LIBJUICE_LIB)" ]; then \
		echo "Building libjuice for $(PLATFORM)..."; \
		echo "  Compiling libjuice sources..."; \
		for src in $(LIBJUICE_DIR)/src/addr.c \
		           $(LIBJUICE_DIR)/src/agent.c \
		           $(LIBJUICE_DIR)/src/base64.c \
		           $(LIBJUICE_DIR)/src/conn.c \
		           $(LIBJUICE_DIR)/src/conn_mux.c \
		           $(LIBJUICE_DIR)/src/conn_poll.c \
		           $(LIBJUICE_DIR)/src/conn_thread.c \
		           $(LIBJUICE_DIR)/src/const_time.c \
		           $(LIBJUICE_DIR)/src/crc32.c \
		           $(LIBJUICE_DIR)/src/hash.c \
		           $(LIBJUICE_DIR)/src/hmac.c \
		           $(LIBJUICE_DIR)/src/ice.c \
		           $(LIBJUICE_DIR)/src/juice.c \
		           $(LIBJUICE_DIR)/src/log.c \
		           $(LIBJUICE_DIR)/src/random.c \
		           $(LIBJUICE_DIR)/src/server.c \
		           $(LIBJUICE_DIR)/src/stun.c \
		           $(LIBJUICE_DIR)/src/tcp.c \
		           $(LIBJUICE_DIR)/src/timestamp.c \
		           $(LIBJUICE_DIR)/src/turn.c \
		           $(LIBJUICE_DIR)/src/udp.c; do \
			obj=$${src%.c}.o; \
			echo "    $$src"; \
			$(CC) $(JUICE_CFLAGS) -c "$$src" -o "$$obj"; \
		done; \
		echo "  Creating libjuice.a..."; \
		$(AR) rcs $(LIBJUICE_LIB) $(LIBJUICE_DIR)/src/*.o; \
		echo "  libjuice built successfully."; \
	fi

clean:
	$(RM) $(TARGET) $(OBJS)

clean-juice-objs:
	$(RM) $(LIBJUICE_DIR)/src/*.o

clean-raylib:
	$(RMDIR) $(RAYLIB_DIR)

clean-libjuice:
	$(RMDIR) $(LIBJUICE_DIR)

clean-all: clean clean-raylib clean-libjuice

# Mesa3D software renderer for Windows (when OpenGL drivers are not available)
# Downloads opengl32.dll from mesa-dist-win GitHub releases
MESA_VERSION = 24.3.4
MESA_RELEASE_URL = https://github.com/pal1000/mesa-dist-win/releases/download/$(MESA_VERSION)/mesa3d-$(MESA_VERSION)-release-msvc.7z

mesa:
ifeq ($(PLATFORM),WINDOWS)
	@echo "Downloading Mesa3D software renderer for Windows..."
	@if command -v curl >/dev/null 2>&1; then \
		curl -L -o mesa3d.7z $(MESA_RELEASE_URL); \
	elif command -v wget >/dev/null 2>&1; then \
		wget -O mesa3d.7z $(MESA_RELEASE_URL); \
	else \
		echo "ERROR: curl or wget required to download Mesa3D"; \
		exit 1; \
	fi
	@echo "Extracting opengl32.dll..."
	@if command -v 7z >/dev/null 2>&1; then \
		7z e mesa3d.7z x64/opengl32.dll -y; \
	elif command -v 7za >/dev/null 2>&1; then \
		7za e mesa3d.7z x64/opengl32.dll -y; \
	else \
		echo "ERROR: 7z or 7za required to extract Mesa3D"; \
		echo "Install with: pacman -S p7zip"; \
		exit 1; \
	fi
	@rm -f mesa3d.7z
	@echo "Mesa3D opengl32.dll extracted successfully!"
	@echo "The game will now use software rendering."
else
	@echo "Mesa3D is only needed for Windows. Your platform: $(PLATFORM)"
endif

clean-mesa:
	$(RM) opengl32.dll mesa3d.7z

help:
	@echo "Chess Game Makefile - Cross-Platform Build System"
	@echo ""
	@echo "Detected Platform: $(PLATFORM)"
	@echo ""
	@echo "Available targets:"
	@echo "  make              - Build raylib, libjuice (if needed) and the game"
	@echo "  make clean        - Remove the chess executable and object files"
	@echo "  make clean-raylib - Remove the raylib directory"
	@echo "  make clean-libjuice - Remove the libjuice directory"
	@echo "  make clean-all    - Remove executable, object files, and libraries"
	@echo "  make mesa         - Download Mesa3D software renderer (Windows only)"
	@echo "  make clean-mesa   - Remove Mesa3D files"
	@echo "  make help         - Show this help message"

