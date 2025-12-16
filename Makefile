# Chess Game Makefile with Local Raylib and libjuice

CC = gcc
CFLAGS = -Wall -Wextra -O2 -I./raylib/src -I./libjuice/include
LDFLAGS = -L./raylib/src -L./libjuice -lraylib -ljuice -lGL -lm -lpthread -ldl -lrt -lX11

TARGET = chess
SRCS = main.c board.c moves.c check.c ui.c menu.c history.c constants.c clock.c network.c multiplayer.c
OBJS = $(SRCS:.c=.o)
HEADERS = types.h board.h moves.h check.h ui.h menu.h history.h clock.h network.h multiplayer.h

RAYLIB_DIR = raylib
RAYLIB_LIB = $(RAYLIB_DIR)/src/libraylib.a

LIBJUICE_DIR = libjuice
LIBJUICE_LIB = $(LIBJUICE_DIR)/libjuice.a

.PHONY: all clean raylib libjuice clean-raylib clean-libjuice clean-all

all: raylib libjuice $(TARGET)

$(TARGET): $(OBJS) $(RAYLIB_LIB) $(LIBJUICE_LIB)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(RAYLIB_LIB) $(LIBJUICE_LIB) -lGL -lm -lpthread -ldl -lrt -lX11

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

raylib: $(RAYLIB_LIB)

$(RAYLIB_LIB):
	@if [ ! -d "$(RAYLIB_DIR)" ]; then \
		echo "Downloading raylib..."; \
		git clone --depth 1 https://github.com/raysan5/raylib.git $(RAYLIB_DIR); \
	fi
	@echo "Building raylib..."
	$(MAKE) -C $(RAYLIB_DIR)/src PLATFORM=PLATFORM_DESKTOP

libjuice: $(LIBJUICE_LIB)

$(LIBJUICE_LIB):
	@if [ ! -d "$(LIBJUICE_DIR)" ]; then \
		echo "Downloading libjuice..."; \
		git clone --depth 1 https://github.com/paullouisageneau/libjuice.git $(LIBJUICE_DIR); \
	fi
	@echo "Building libjuice..."
	$(MAKE) -C $(LIBJUICE_DIR)

clean:
	rm -f $(TARGET) $(OBJS)

clean-raylib:
	rm -rf $(RAYLIB_DIR)

clean-libjuice:
	rm -rf $(LIBJUICE_DIR)

clean-all: clean clean-raylib clean-libjuice

