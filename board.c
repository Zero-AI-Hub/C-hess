/**
 * Chess Game - Board Management
 * Board state and basic query operations.
 */

#include "board.h"
#include "history.h"
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

// Promotion move tracking for history recording
Position promotionFromPos = {-1, -1};
bool promotionWasCapture = false;

// Cached king positions for optimization
Position whiteKingPos = {7, 4};
Position blackKingPos = {0, 4};

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
  selectedPos = INVALID_POS;
  enPassantTarget = INVALID_POS;
  enPassantPawn = INVALID_POS;
  gameState = GAME_PLAYING;

  // Reset drag state
  isDragging = false;
  dragStartPos = INVALID_POS;
  dragOffset = (Vector2){0, 0};

  // Reset promotion state
  promotionPos = INVALID_POS;
  promotionFromPos = INVALID_POS;
  promotionWasCapture = false;

  // Reset cached king positions
  whiteKingPos = (Position){7, 4};
  blackKingPos = (Position){0, 4};

  InitMoveHistory();
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
  // Return cached position for optimization
  return (color == COLOR_WHITE) ? whiteKingPos : blackKingPos;
}
