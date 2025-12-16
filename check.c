/**
 * Chess Game - Check Detection
 * Check, checkmate, and stalemate detection.
 */

#include "check.h"
#include "board.h"
#include "moves.h"

//==============================================================================
// ATTACK DETECTION
//==============================================================================

bool IsSquareAttacked(int row, int col, PieceColor byColor) {
  // Pawn attacks (pawns attack diagonally)
  int pawnDir = (byColor == COLOR_WHITE) ? 1 : -1;
  for (int dc = -1; dc <= 1; dc += 2) {
    int pawnRow = row + pawnDir;
    int pawnCol = col + dc;
    if (IsValidPosition(pawnRow, pawnCol) &&
        board[pawnRow][pawnCol].type == PIECE_PAWN &&
        board[pawnRow][pawnCol].color == byColor) {
      return true;
    }
  }

  // Knight attacks
  for (int i = 0; i < 8; i++) {
    int targetRow = row + KNIGHT_MOVES[i][0];
    int targetCol = col + KNIGHT_MOVES[i][1];
    if (IsValidPosition(targetRow, targetCol) &&
        board[targetRow][targetCol].type == PIECE_KNIGHT &&
        board[targetRow][targetCol].color == byColor) {
      return true;
    }
  }

  // King attacks (adjacent squares)
  for (int dr = -1; dr <= 1; dr++) {
    for (int dc = -1; dc <= 1; dc++) {
      if (dr == 0 && dc == 0)
        continue;
      int targetRow = row + dr;
      int targetCol = col + dc;
      if (IsValidPosition(targetRow, targetCol) &&
          board[targetRow][targetCol].type == PIECE_KING &&
          board[targetRow][targetCol].color == byColor) {
        return true;
      }
    }
  }

  // Rook/Queen attacks (horizontal/vertical)
  for (int d = 0; d < 4; d++) {
    for (int i = 1; i < BOARD_SIZE; i++) {
      int targetRow = row + i * ROOK_DIRECTIONS[d][0];
      int targetCol = col + i * ROOK_DIRECTIONS[d][1];
      if (!IsValidPosition(targetRow, targetCol))
        break;
      if (board[targetRow][targetCol].type != PIECE_NONE) {
        if (board[targetRow][targetCol].color == byColor &&
            (board[targetRow][targetCol].type == PIECE_ROOK ||
             board[targetRow][targetCol].type == PIECE_QUEEN)) {
          return true;
        }
        break;
      }
    }
  }

  // Bishop/Queen attacks (diagonal)
  for (int d = 0; d < 4; d++) {
    for (int i = 1; i < BOARD_SIZE; i++) {
      int targetRow = row + i * BISHOP_DIRECTIONS[d][0];
      int targetCol = col + i * BISHOP_DIRECTIONS[d][1];
      if (!IsValidPosition(targetRow, targetCol))
        break;
      if (board[targetRow][targetCol].type != PIECE_NONE) {
        if (board[targetRow][targetCol].color == byColor &&
            (board[targetRow][targetCol].type == PIECE_BISHOP ||
             board[targetRow][targetCol].type == PIECE_QUEEN)) {
          return true;
        }
        break;
      }
    }
  }

  return false;
}

bool IsInCheck(PieceColor color) {
  Position king = FindKing(color);
  if (king.row == -1)
    return false;
  return IsSquareAttacked(king.row, king.col, OPPONENT_COLOR(color));
}

//==============================================================================
// MOVE LEGALITY CHECK
//==============================================================================

bool WouldBeInCheck(int fromRow, int fromCol, int toRow, int toCol,
                    PieceColor color) {
  // Save board state
  Piece movingPiece = board[fromRow][fromCol];
  Piece capturedPiece = board[toRow][toCol];
  Piece enPassantCaptured = {PIECE_NONE, COLOR_NONE, false};

  // Handle en passant capture simulation
  bool isEnPassant =
      (movingPiece.type == PIECE_PAWN && toRow == enPassantTarget.row &&
       toCol == enPassantTarget.col);
  if (isEnPassant) {
    enPassantCaptured = board[enPassantPawn.row][enPassantPawn.col];
    board[enPassantPawn.row][enPassantPawn.col] =
        (Piece){PIECE_NONE, COLOR_NONE, false};
  }

  // Make temporary move
  board[toRow][toCol] = movingPiece;
  board[fromRow][fromCol] = (Piece){PIECE_NONE, COLOR_NONE, false};

  bool inCheck = IsInCheck(color);

  // Restore board state
  board[fromRow][fromCol] = movingPiece;
  board[toRow][toCol] = capturedPiece;
  if (isEnPassant) {
    board[enPassantPawn.row][enPassantPawn.col] = enPassantCaptured;
  }

  return inCheck;
}

//==============================================================================
// CHECKMATE/STALEMATE DETECTION
//==============================================================================

bool HasLegalMoves(PieceColor color) {
  Position savedSelected = selectedPos;

  for (int row = 0; row < BOARD_SIZE; row++) {
    for (int col = 0; col < BOARD_SIZE; col++) {
      if (board[row][col].type == PIECE_NONE || board[row][col].color != color)
        continue;

      selectedPos = (Position){row, col};
      ClearValidMoves();
      CalculateValidMoves(row, col);

      // Check if any valid move exists
      for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
          if (IsValidMove(r, c)) {
            selectedPos = savedSelected;
            ClearValidMoves();
            return true;
          }
        }
      }
    }
  }

  selectedPos = savedSelected;
  ClearValidMoves();
  return false;
}

void UpdateGameState(void) {
  bool inCheck = IsInCheck(currentTurn);
  bool hasLegalMoves = HasLegalMoves(currentTurn);

  if (!hasLegalMoves) {
    gameState = inCheck ? GAME_CHECKMATE : GAME_STALEMATE;
  } else {
    gameState = inCheck ? GAME_CHECK : GAME_PLAYING;
  }
}
