# Chess Game Makefile with Local Raylib

CC = gcc
CFLAGS = -Wall -Wextra -O2 -I./raylib/src
LDFLAGS = -L./raylib/src -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

TARGET = chess
SRCS = main.c board.c moves.c check.c ui.c menu.c history.c constants.c clock.c
OBJS = $(SRCS:.c=.o)
HEADERS = types.h board.h moves.h check.h ui.h menu.h history.h clock.h

RAYLIB_DIR = raylib
RAYLIB_LIB = $(RAYLIB_DIR)/src/libraylib.a

.PHONY: all clean raylib clean-raylib clean-all

all: raylib $(TARGET)

$(TARGET): $(OBJS) $(RAYLIB_LIB)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(RAYLIB_LIB) -lGL -lm -lpthread -ldl -lrt -lX11

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

clean:
	rm -f $(TARGET) $(OBJS)

clean-raylib:
	rm -rf $(RAYLIB_DIR)

clean-all: clean clean-raylib
