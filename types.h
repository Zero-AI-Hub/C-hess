/**
 * Chess Game - Type Definitions and Constants
 * Shared types, enums, and constants used across all modules.
 */

#ifndef TYPES_H
#define TYPES_H

#include "raylib.h"
#include <stdbool.h>

//==============================================================================
// BOARD CONSTANTS
//==============================================================================

#define BOARD_SIZE 8
#define TILE_SIZE 80
#define BOARD_OFFSET_X 40
#define BOARD_OFFSET_Y 40
#define HISTORY_PANEL_WIDTH 250
#define WINDOW_WIDTH                                                           \
  (BOARD_SIZE * TILE_SIZE + BOARD_OFFSET_X * 2 + HISTORY_PANEL_WIDTH)
#define WINDOW_HEIGHT (BOARD_SIZE * TILE_SIZE + BOARD_OFFSET_Y * 2 + 60)

//==============================================================================
// MOVE HISTORY CONSTANTS
//==============================================================================

#define MAX_MOVES 256
#define MOVE_NOTATION_LEN 12

//==============================================================================
// SPRITE CONFIGURATION
//==============================================================================

#define SPRITE_SIZE 60
#define SPRITE_COLS 6
#define SPRITE_ROWS 2

//==============================================================================
// UI CONSTANTS
//==============================================================================

#define VALID_MOVE_RADIUS 15
#define PANEL_PADDING 20
#define BUTTON_SPACING 10
#define FONT_SIZE_SMALL 20
#define FONT_SIZE_MEDIUM 24
#define FONT_SIZE_LARGE 32
#define FONT_SIZE_TITLE 48
#define FONT_SIZE_TITLE_LARGE 80
#define MENU_BUTTON_WIDTH 200
#define MENU_BUTTON_HEIGHT 50
#define MENU_BUTTON_Y_START 380
#define MENU_BUTTON_Y_SPACING 70

// Clock display constants
#define CLOCK_PANEL_WIDTH 200
#define CLOCK_PANEL_HEIGHT 50
#define CLOCK_LOW_TIME_THRESHOLD 20.0f
#define CLOCK_CRITICAL_TIME_THRESHOLD 10.0f

//==============================================================================
// COLOR PALETTE (defined in constants.c)
//==============================================================================

extern const Color COLOR_LIGHT_SQUARE;
extern const Color COLOR_DARK_SQUARE;
extern const Color COLOR_SELECTED;
extern const Color COLOR_CHECK_HIGHLIGHT;
extern const Color COLOR_VALID_MOVE;
extern const Color COLOR_CAPTURE;
extern const Color COLOR_BACKGROUND;
extern const Color COLOR_OVERLAY_DARK;
extern const Color COLOR_OVERLAY_DARKER;
extern const Color COLOR_PANEL_BG;
extern const Color COLOR_BUTTON_BG;
extern const Color COLOR_BUTTON_HOVER;
extern const Color COLOR_TITLE_GOLD;
extern const Color COLOR_TITLE_SHADOW;

//==============================================================================
// PIECE TYPES AND ENUMS
//==============================================================================

typedef enum {
  PIECE_NONE = 0,
  PIECE_KING,
  PIECE_QUEEN,
  PIECE_BISHOP,
  PIECE_KNIGHT,
  PIECE_ROOK,
  PIECE_PAWN
} PieceType;

typedef enum { COLOR_NONE = 0, COLOR_WHITE, COLOR_BLACK } PieceColor;

typedef struct {
  PieceType type;
  PieceColor color;
  bool hasMoved;
} Piece;

typedef struct {
  int row;
  int col;
} Position;

typedef enum {
  GAME_PLAYING,
  GAME_CHECK,
  GAME_CHECKMATE,
  GAME_STALEMATE,
  GAME_TIMEOUT,
  GAME_PROMOTING
} GameState;

typedef enum {
  SCREEN_TITLE,
  SCREEN_CLOCK_SETUP,
  SCREEN_GAME,
  SCREEN_OPTIONS,
  SCREEN_MULTIPLAYER,
  SCREEN_MP_HOST,
  SCREEN_MP_JOIN
} ScreenState;

// Helper struct for UI panel layouts
typedef struct {
  int x, y, width, height;
} PanelLayout;

//==============================================================================
// HELPER MACROS
//==============================================================================

#define OPPONENT_COLOR(c) ((c) == COLOR_WHITE ? COLOR_BLACK : COLOR_WHITE)
#define INVALID_POS ((Position){-1, -1})
#define EMPTY_SQUARE ((Piece){PIECE_NONE, COLOR_NONE, false})

//==============================================================================
// MOVEMENT PATTERNS (defined in constants.c)
//==============================================================================

extern const int ROOK_DIRECTIONS[4][2];
extern const int BISHOP_DIRECTIONS[4][2];
extern const int KNIGHT_MOVES[8][2];

#endif // TYPES_H
