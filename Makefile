# Chess Game Makefile with Local Raylib and libjuice
# Cross-platform support for Linux and Windows (MinGW/MSYS2)

CC = gcc
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
# Note: MSYS2/MinGW sets OS=Windows_NT but uses bash shell
ifeq ($(OS),Windows_NT)
    PLATFORM = WINDOWS
    TARGET := $(TARGET).exe
    LDFLAGS = -L./raylib/src -L./libjuice -lraylib -ljuice -lopengl32 -lgdi32 -lwinmm -lws2_32 -lbcrypt -static -lpthread
    # Windows-specific: need to link ws2_32 for libjuice sockets
    LIBJUICE_EXTRA = -lws2_32 -lbcrypt
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        PLATFORM = LINUX
        LDFLAGS = -L./raylib/src -L./libjuice -lraylib -ljuice -lGL -lm -lpthread -ldl -lrt -lX11
        LIBJUICE_EXTRA =
    else ifeq ($(UNAME_S),Darwin)
        PLATFORM = MACOS
        LDFLAGS = -L./raylib/src -L./libjuice -lraylib -ljuice -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo -lpthread
        LIBJUICE_EXTRA =
    else
        # MSYS2/MinGW reports MINGW64_NT-* or similar
        ifneq (,$(findstring MINGW,$(UNAME_S)))
            PLATFORM = WINDOWS
            TARGET := $(TARGET).exe
            LDFLAGS = -L./raylib/src -L./libjuice -lraylib -ljuice -lopengl32 -lgdi32 -lwinmm -lws2_32 -lbcrypt -static -lpthread
            LIBJUICE_EXTRA = -lws2_32 -lbcrypt
        else ifneq (,$(findstring MSYS,$(UNAME_S)))
            PLATFORM = WINDOWS
            TARGET := $(TARGET).exe
            LDFLAGS = -L./raylib/src -L./libjuice -lraylib -ljuice -lopengl32 -lgdi32 -lwinmm -lws2_32 -lbcrypt -static -lpthread
            LIBJUICE_EXTRA = -lws2_32 -lbcrypt
        endif
    endif
endif

# Always use Unix-style commands (MSYS2/MinGW uses bash)
RM = rm -f
RMDIR = rm -rf

.PHONY: all clean raylib libjuice clean-raylib clean-libjuice clean-all help

all: raylib libjuice $(TARGET)

$(TARGET): $(OBJS) $(RAYLIB_LIB) $(LIBJUICE_LIB)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(RAYLIB_LIB) $(LIBJUICE_LIB) $(LDFLAGS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

raylib: $(RAYLIB_LIB)

$(RAYLIB_LIB):
	@if [ ! -d "$(RAYLIB_DIR)" ]; then \
		echo "Downloading raylib..."; \
		git clone --depth 1 https://github.com/raysan5/raylib.git $(RAYLIB_DIR); \
	fi
	@echo "Building raylib for $(PLATFORM)..."
	$(MAKE) -C $(RAYLIB_DIR)/src PLATFORM=PLATFORM_DESKTOP

libjuice: $(LIBJUICE_LIB)

# Build libjuice manually for better cross-platform support
JUICE_SRCS = $(wildcard $(LIBJUICE_DIR)/src/*.c)
JUICE_OBJS = $(JUICE_SRCS:.c=.o)
JUICE_CFLAGS = -O2 -fPIC -fvisibility=hidden -DJUICE_EXPORTS -DUSE_NETTLE=0 -I$(LIBJUICE_DIR)/include/juice

$(LIBJUICE_LIB): $(LIBJUICE_DIR)
	@if [ ! -f "$(LIBJUICE_LIB)" ]; then \
		echo "Building libjuice for $(PLATFORM)..."; \
		for src in $(LIBJUICE_DIR)/src/*.c; do \
			obj=$${src%.c}.o; \
			echo "  Compiling $$src..."; \
			$(CC) $(JUICE_CFLAGS) -c "$$src" -o "$$obj"; \
		done; \
		$(AR) rcs $(LIBJUICE_LIB) $(LIBJUICE_DIR)/src/*.o; \
		echo "libjuice built successfully."; \
	fi

$(LIBJUICE_DIR):
	@if [ ! -d "$(LIBJUICE_DIR)" ]; then \
		echo "Downloading libjuice..."; \
		git clone --depth 1 https://github.com/paullouisageneau/libjuice.git $(LIBJUICE_DIR); \
	fi

clean:
	$(RM) $(TARGET) $(OBJS)

clean-raylib:
	$(RMDIR) $(RAYLIB_DIR)

clean-libjuice:
	$(RMDIR) $(LIBJUICE_DIR)

clean-all: clean clean-raylib clean-libjuice

help:
	@echo "Chess Game Makefile - Cross-Platform Build System"
	@echo ""
	@echo "Detected Platform: $(PLATFORM)"
	@echo ""
	@echo "Available targets:"
	@echo "  make          - Build raylib, libjuice (if needed) and the game"
	@echo "  make clean    - Remove the chess executable and object files"
	@echo "  make clean-raylib   - Remove the raylib directory"
	@echo "  make clean-libjuice - Remove the libjuice directory"
	@echo "  make clean-all      - Remove executable, object files, and libraries"
	@echo "  make help     - Show this help message"
