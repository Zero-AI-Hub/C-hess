/**
 * Chess Game - Board Management
 * Board state and basic query operations.
 */

#include "board.h"
#include <string.h>

//==============================================================================
// GLOBAL STATE DEFINITIONS
//==============================================================================

Piece board[BOARD_SIZE][BOARD_SIZE];
PieceColor currentTurn = COLOR_WHITE;
Position selectedPos = {-1, -1};
GameState gameState = GAME_PLAYING;
ScreenState currentScreen = SCREEN_TITLE;
Position enPassantTarget = {-1, -1};
Position enPassantPawn = {-1, -1};
Position promotionPos = {-1, -1};
bool isDragging = false;
Position dragStartPos = {-1, -1};
Vector2 dragOffset = {0, 0};

//==============================================================================
// BOARD INITIALIZATION
//==============================================================================

void InitBoard(void) {
  memset(board, 0, sizeof(board));

  // Piece types for back row: Rook, Knight, Bishop, Queen, King, Bishop,
  // Knight, Rook
  static const PieceType backRow[8] = {PIECE_ROOK,   PIECE_KNIGHT, PIECE_BISHOP,
                                       PIECE_QUEEN,  PIECE_KING,   PIECE_BISHOP,
                                       PIECE_KNIGHT, PIECE_ROOK};

  for (int i = 0; i < BOARD_SIZE; i++) {
    // Black pieces (top)
    board[0][i] = (Piece){backRow[i], COLOR_BLACK, false};
    board[1][i] = (Piece){PIECE_PAWN, COLOR_BLACK, false};
    // White pieces (bottom)
    board[6][i] = (Piece){PIECE_PAWN, COLOR_WHITE, false};
    board[7][i] = (Piece){backRow[i], COLOR_WHITE, false};
  }

  // Reset game state
  currentTurn = COLOR_WHITE;
  selectedPos = (Position){-1, -1};
  enPassantTarget = (Position){-1, -1};
  enPassantPawn = (Position){-1, -1};
  gameState = GAME_PLAYING;
}

//==============================================================================
// POSITION QUERIES
//==============================================================================

bool IsValidPosition(int row, int col) {
  return row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE;
}

bool IsEmpty(int row, int col) { return board[row][col].type == PIECE_NONE; }

bool IsEnemy(int row, int col, PieceColor color) {
  return board[row][col].type != PIECE_NONE && board[row][col].color != color;
}

bool IsAlly(int row, int col, PieceColor color) {
  return board[row][col].type != PIECE_NONE && board[row][col].color == color;
}

Position FindKing(PieceColor color) {
  for (int row = 0; row < BOARD_SIZE; row++) {
    for (int col = 0; col < BOARD_SIZE; col++) {
      if (board[row][col].type == PIECE_KING &&
          board[row][col].color == color) {
        return (Position){row, col};
      }
    }
  }
  return (Position){-1, -1};
}
