/**
 * Chess Game - Move Validation
 * Move calculation and execution.
 */

#include "moves.h"
#include "board.h"
#include "check.h"
#include "history.h"
#include <stdlib.h>
#include <string.h>

//==============================================================================
// VALID MOVES STATE
//==============================================================================

bool validMoves[BOARD_SIZE][BOARD_SIZE];

//==============================================================================
// MOVE HELPERS
//==============================================================================

void ClearValidMoves(void) { memset(validMoves, false, sizeof(validMoves)); }

bool IsValidMove(int row, int col) { return validMoves[row][col]; }

void AddMoveIfValid(int row, int col, PieceColor color) {
  if (IsValidPosition(row, col) && !IsAlly(row, col, color)) {
    if (!WouldBeInCheck(selectedPos.row, selectedPos.col, row, col, color)) {
      validMoves[row][col] = true;
    }
  }
}

//==============================================================================
// PIECE-SPECIFIC MOVE CALCULATION
//==============================================================================

static void CalculatePawnMoves(int row, int col, PieceColor color) {
  int direction = (color == COLOR_WHITE) ? -1 : 1;
  int startRow = (color == COLOR_WHITE) ? 6 : 1;

  // Forward one square
  if (IsValidPosition(row + direction, col) && IsEmpty(row + direction, col)) {
    AddMoveIfValid(row + direction, col, color);

    // Forward two squares from starting position
    if (row == startRow && IsEmpty(row + 2 * direction, col)) {
      AddMoveIfValid(row + 2 * direction, col, color);
    }
  }

  // Diagonal captures
  for (int dc = -1; dc <= 1; dc += 2) {
    if (IsValidPosition(row + direction, col + dc) &&
        IsEnemy(row + direction, col + dc, color)) {
      AddMoveIfValid(row + direction, col + dc, color);
    }
  }

  // En passant capture
  if (enPassantTarget.row != -1 && row + direction == enPassantTarget.row &&
      abs(col - enPassantTarget.col) == 1 && enPassantPawn.row == row &&
      abs(enPassantPawn.col - col) == 1) {
    AddMoveIfValid(enPassantTarget.row, enPassantTarget.col, color);
  }
}

static void CalculateSlidingMoves(int row, int col, PieceColor color,
                                  const int dirs[4][2]) {
  for (int d = 0; d < 4; d++) {
    for (int i = 1; i < BOARD_SIZE; i++) {
      int targetRow = row + i * dirs[d][0];
      int targetCol = col + i * dirs[d][1];

      if (!IsValidPosition(targetRow, targetCol))
        break;
      if (IsAlly(targetRow, targetCol, color))
        break;

      AddMoveIfValid(targetRow, targetCol, color);

      if (IsEnemy(targetRow, targetCol, color))
        break;
    }
  }
}

static void CalculateKnightMoves(int row, int col, PieceColor color) {
  for (int i = 0; i < 8; i++) {
    AddMoveIfValid(row + KNIGHT_MOVES[i][0], col + KNIGHT_MOVES[i][1], color);
  }
}

static void CalculateKingMoves(int row, int col, PieceColor color) {
  // Normal moves (8 adjacent squares)
  for (int dr = -1; dr <= 1; dr++) {
    for (int dc = -1; dc <= 1; dc++) {
      if (dr == 0 && dc == 0)
        continue;
      AddMoveIfValid(row + dr, col + dc, color);
    }
  }

  // Castling: king and rook haven't moved, not in check, squares clear and safe
  if (board[row][col].hasMoved || IsInCheck(color))
    return;

  PieceColor enemy = OPPONENT_COLOR(color);

  // Kingside castling (O-O)
  if (board[row][7].type == PIECE_ROOK && !board[row][7].hasMoved &&
      IsEmpty(row, 5) && IsEmpty(row, 6) && !IsSquareAttacked(row, 5, enemy) &&
      !IsSquareAttacked(row, 6, enemy)) {
    AddMoveIfValid(row, 6, color);
  }

  // Queenside castling (O-O-O)
  if (board[row][0].type == PIECE_ROOK && !board[row][0].hasMoved &&
      IsEmpty(row, 1) && IsEmpty(row, 2) && IsEmpty(row, 3) &&
      !IsSquareAttacked(row, 2, enemy) && !IsSquareAttacked(row, 3, enemy)) {
    AddMoveIfValid(row, 2, color);
  }
}

//==============================================================================
// MAIN MOVE CALCULATION
//==============================================================================

void CalculateValidMoves(int row, int col) {
  Piece piece = board[row][col];

  switch (piece.type) {
  case PIECE_PAWN:
    CalculatePawnMoves(row, col, piece.color);
    break;
  case PIECE_ROOK:
    CalculateSlidingMoves(row, col, piece.color, ROOK_DIRECTIONS);
    break;
  case PIECE_KNIGHT:
    CalculateKnightMoves(row, col, piece.color);
    break;
  case PIECE_BISHOP:
    CalculateSlidingMoves(row, col, piece.color, BISHOP_DIRECTIONS);
    break;
  case PIECE_QUEEN:
    CalculateSlidingMoves(row, col, piece.color, ROOK_DIRECTIONS);
    CalculateSlidingMoves(row, col, piece.color, BISHOP_DIRECTIONS);
    break;
  case PIECE_KING:
    CalculateKingMoves(row, col, piece.color);
    break;
  default:
    break;
  }
}

//==============================================================================
// MOVE EXECUTION
//==============================================================================

void MovePiece(int toRow, int toCol) {
  int fromRow = selectedPos.row;
  int fromCol = selectedPos.col;
  Piece piece = board[fromRow][fromCol];

  // Determine move properties for history recording
  bool isCapture = board[toRow][toCol].type != PIECE_NONE;
  bool isCastleKingside = false;
  bool isCastleQueenside = false;
  bool isEnPassantCapture = false;

  // Check for castling
  if (piece.type == PIECE_KING && abs(toCol - fromCol) == 2) {
    if (toCol > fromCol) {
      isCastleKingside = true;
    } else {
      isCastleQueenside = true;
    }
  }

  // Check for en passant
  if (piece.type == PIECE_PAWN && toRow == enPassantTarget.row &&
      toCol == enPassantTarget.col) {
    isEnPassantCapture = true;
    isCapture = true;
  }

  // Save and reset en passant state
  Position oldEnPassantTarget = enPassantTarget;
  Position oldEnPassantPawn = enPassantPawn;
  enPassantTarget = (Position){-1, -1};
  enPassantPawn = (Position){-1, -1};

  // Handle en passant capture: remove the captured pawn
  if (piece.type == PIECE_PAWN && toRow == oldEnPassantTarget.row &&
      toCol == oldEnPassantTarget.col) {
    board[oldEnPassantPawn.row][oldEnPassantPawn.col] =
        (Piece){PIECE_NONE, COLOR_NONE, false};
  }

  // Handle castling: move the rook alongside the king
  if (piece.type == PIECE_KING && abs(toCol - fromCol) == 2) {
    if (toCol > fromCol) {
      // Kingside castling - move rook from h-file to f-file
      board[fromRow][5] = board[fromRow][7];
      board[fromRow][5].hasMoved = true;
      board[fromRow][7] = (Piece){PIECE_NONE, COLOR_NONE, false};
    } else {
      // Queenside castling - move rook from a-file to d-file
      board[fromRow][3] = board[fromRow][0];
      board[fromRow][3].hasMoved = true;
      board[fromRow][0] = (Piece){PIECE_NONE, COLOR_NONE, false};
    }
  }

  // Set en passant target if pawn moves two squares
  if (piece.type == PIECE_PAWN && abs(toRow - fromRow) == 2) {
    enPassantTarget = (Position){(fromRow + toRow) / 2, fromCol};
    enPassantPawn = (Position){toRow, toCol};
  }

  // Execute the move
  board[toRow][toCol] = piece;
  board[toRow][toCol].hasMoved = true;
  board[fromRow][fromCol] = (Piece){PIECE_NONE, COLOR_NONE, false};

  // Check for pawn promotion
  if (piece.type == PIECE_PAWN &&
      ((piece.color == COLOR_WHITE && toRow == 0) ||
       (piece.color == COLOR_BLACK && toRow == 7))) {
    // Save move info for recording after promotion choice
    promotionFromPos = (Position){fromRow, fromCol};
    promotionWasCapture = isCapture;
    promotionPos = (Position){toRow, toCol};
    gameState = GAME_PROMOTING;
    selectedPos = (Position){-1, -1};
    ClearValidMoves();
    return;
  }

  // Record the move for history
  RecordMove(fromRow, fromCol, toRow, toCol, piece.type, piece.color, isCapture,
             isCastleKingside, isCastleQueenside, isEnPassantCapture, false,
             PIECE_NONE);

  // Switch turns and update game state
  currentTurn = OPPONENT_COLOR(currentTurn);
  selectedPos = (Position){-1, -1};
  ClearValidMoves();
  UpdateGameState();

  // Update move history with check/checkmate status
  UpdateLastMoveStatus(gameState == GAME_CHECK || gameState == GAME_CHECKMATE,
                       gameState == GAME_CHECKMATE);
}
