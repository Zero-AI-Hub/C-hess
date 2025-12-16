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

//==============================================================================
// COLOR PALETTE
//==============================================================================

static const Color COLOR_LIGHT_SQUARE = {240, 217, 181, 255};
static const Color COLOR_DARK_SQUARE = {181, 136, 99, 255};
static const Color COLOR_SELECTED = {255, 255, 0, 100};
static const Color COLOR_CHECK_HIGHLIGHT = {255, 0, 0, 150};
static const Color COLOR_VALID_MOVE = {0, 255, 0, 80};
static const Color COLOR_CAPTURE = {255, 0, 0, 80};
static const Color COLOR_BACKGROUND = {40, 40, 40, 255};
static const Color COLOR_OVERLAY_DARK = {0, 0, 0, 150};
static const Color COLOR_OVERLAY_DARKER = {0, 0, 0, 180};
static const Color COLOR_PANEL_BG = {60, 60, 60, 255};
static const Color COLOR_BUTTON_BG = {100, 100, 100, 255};
static const Color COLOR_BUTTON_HOVER = {130, 130, 130, 255};
static const Color COLOR_TITLE_GOLD = {255, 215, 0, 255};
static const Color COLOR_TITLE_SHADOW = {80, 60, 0, 255};

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
  GAME_PROMOTING
} GameState;

typedef enum { SCREEN_TITLE, SCREEN_GAME, SCREEN_OPTIONS } ScreenState;

// Helper struct for UI panel layouts
typedef struct {
  int x, y, width, height;
} PanelLayout;

//==============================================================================
// HELPER MACROS
//==============================================================================

#define OPPONENT_COLOR(c) ((c) == COLOR_WHITE ? COLOR_BLACK : COLOR_WHITE)

//==============================================================================
// MOVEMENT PATTERNS
//==============================================================================

static const int ROOK_DIRECTIONS[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
static const int BISHOP_DIRECTIONS[4][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
static const int KNIGHT_MOVES[8][2] = {{-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
                                       {1, -2},  {1, 2},  {2, -1},  {2, 1}};

#endif // TYPES_H
