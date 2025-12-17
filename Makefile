# Chess Game Makefile with Local Raylib and libjuice
# Cross-platform support for Linux and Windows (MinGW)

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
ifeq ($(OS),Windows_NT)
    # Windows (MinGW)
    PLATFORM = WINDOWS
    TARGET := $(TARGET).exe
    LDFLAGS = -L./raylib/src -L./libjuice -lraylib -ljuice -lopengl32 -lgdi32 -lwinmm -lws2_32 -lbcrypt -static -lpthread
    RAYLIB_PLATFORM = PLATFORM_DESKTOP
    RM = del /Q
    RMDIR = rmdir /S /Q
    MKDIR = if not exist $1 mkdir $1
    SHELL_CHECK = if not exist
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        # Linux
        PLATFORM = LINUX
        LDFLAGS = -L./raylib/src -L./libjuice -lraylib -ljuice -lGL -lm -lpthread -ldl -lrt -lX11
        RAYLIB_PLATFORM = PLATFORM_DESKTOP
        RM = rm -f
        RMDIR = rm -rf
        MKDIR = mkdir -p $1
        SHELL_CHECK = test ! -d
    else ifeq ($(UNAME_S),Darwin)
        # macOS (bonus support)
        PLATFORM = MACOS
        LDFLAGS = -L./raylib/src -L./libjuice -lraylib -ljuice -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo -lpthread
        RAYLIB_PLATFORM = PLATFORM_DESKTOP
        RM = rm -f
        RMDIR = rm -rf
        MKDIR = mkdir -p $1
        SHELL_CHECK = test ! -d
    endif
endif

.PHONY: all clean raylib libjuice clean-raylib clean-libjuice clean-all help

all: raylib libjuice $(TARGET)

$(TARGET): $(OBJS) $(RAYLIB_LIB) $(LIBJUICE_LIB)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(RAYLIB_LIB) $(LIBJUICE_LIB) $(LDFLAGS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

raylib: $(RAYLIB_LIB)

$(RAYLIB_LIB):
ifeq ($(OS),Windows_NT)
	@if not exist "$(RAYLIB_DIR)" ( \
		echo Downloading raylib... && \
		git clone --depth 1 https://github.com/raysan5/raylib.git $(RAYLIB_DIR) \
	)
	@echo Building raylib for Windows...
	$(MAKE) -C $(RAYLIB_DIR)/src PLATFORM=$(RAYLIB_PLATFORM) CC=gcc
else
	@if [ ! -d "$(RAYLIB_DIR)" ]; then \
		echo "Downloading raylib..."; \
		git clone --depth 1 https://github.com/raysan5/raylib.git $(RAYLIB_DIR); \
	fi
	@echo "Building raylib for $(PLATFORM)..."
	$(MAKE) -C $(RAYLIB_DIR)/src PLATFORM=$(RAYLIB_PLATFORM)
endif

libjuice: $(LIBJUICE_LIB)

$(LIBJUICE_LIB):
ifeq ($(OS),Windows_NT)
	@if not exist "$(LIBJUICE_DIR)" ( \
		echo Downloading libjuice... && \
		git clone --depth 1 https://github.com/paullouisageneau/libjuice.git $(LIBJUICE_DIR) \
	)
	@echo Building libjuice for Windows...
	$(MAKE) -C $(LIBJUICE_DIR) CC=gcc
else
	@if [ ! -d "$(LIBJUICE_DIR)" ]; then \
		echo "Downloading libjuice..."; \
		git clone --depth 1 https://github.com/paullouisageneau/libjuice.git $(LIBJUICE_DIR); \
	fi
	@echo "Building libjuice for $(PLATFORM)..."
	$(MAKE) -C $(LIBJUICE_DIR)
endif

clean:
ifeq ($(OS),Windows_NT)
	$(RM) $(TARGET) *.o 2>nul || (exit 0)
else
	$(RM) $(TARGET) $(OBJS)
endif

clean-raylib:
ifeq ($(OS),Windows_NT)
	@if exist "$(RAYLIB_DIR)" $(RMDIR) $(RAYLIB_DIR)
else
	$(RMDIR) $(RAYLIB_DIR)
endif

clean-libjuice:
ifeq ($(OS),Windows_NT)
	@if exist "$(LIBJUICE_DIR)" $(RMDIR) $(LIBJUICE_DIR)
else
	$(RMDIR) $(LIBJUICE_DIR)
endif

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
