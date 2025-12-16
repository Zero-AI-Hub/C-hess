# Chess Game Makefile with Local Raylib

CC = gcc
CFLAGS = -Wall -Wextra -O2 -I./raylib/src
LDFLAGS = -L./raylib/src -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

TARGET = chess
SRC = main.c

RAYLIB_DIR = raylib
RAYLIB_LIB = $(RAYLIB_DIR)/src/libraylib.a

.PHONY: all clean raylib clean-raylib

all: raylib $(TARGET)

$(TARGET): $(SRC) $(RAYLIB_LIB)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(RAYLIB_LIB) -lGL -lm -lpthread -ldl -lrt -lX11

raylib: $(RAYLIB_LIB)

$(RAYLIB_LIB):
	@if [ ! -d "$(RAYLIB_DIR)" ]; then \
		echo "Downloading raylib..."; \
		git clone --depth 1 https://github.com/raysan5/raylib.git $(RAYLIB_DIR); \
	fi
	@echo "Building raylib..."
	$(MAKE) -C $(RAYLIB_DIR)/src PLATFORM=PLATFORM_DESKTOP

clean:
	rm -f $(TARGET)

clean-raylib:
	rm -rf $(RAYLIB_DIR)

clean-all: clean clean-raylib
