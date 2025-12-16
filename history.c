/**
 * Chess Game - Move History
 * Recording and displaying moves in algebraic notation.
 */

#include "history.h"
#include "board.h"
#include <stdlib.h>
#include <string.h>

//==============================================================================
// GLOBAL STATE DEFINITIONS
//==============================================================================

MoveRecord moveHistory[MAX_MOVES];
int moveCount = 0;
int historyScrollOffset = 0;

//==============================================================================
// HISTORY MANAGEMENT
//==============================================================================

void InitMoveHistory(void) {
  moveCount = 0;
  historyScrollOffset = 0;
  memset(moveHistory, 0, sizeof(moveHistory));
}

int GetMoveCount(void) { return moveCount; }

//==============================================================================
// NOTATION GENERATION
//==============================================================================

static char ColToFile(int col) { return 'a' + col; }

static char RowToRank(int row) { return '8' - row; }

static char PieceToChar(PieceType type) {
  switch (type) {
  case PIECE_KING:
    return 'K';
  case PIECE_QUEEN:
    return 'Q';
  case PIECE_ROOK:
    return 'R';
  case PIECE_BISHOP:
    return 'B';
  case PIECE_KNIGHT:
    return 'N';
  default:
    return '\0';
  }
}

static bool CanPieceReachSquare(int fromRow, int fromCol, int toRow, int toCol,
                                PieceType type) {
  // Check if a piece of the given type at (fromRow, fromCol) can reach (toRow,
  // toCol) This is a simplified check that verifies the move pattern is valid

  int dRow = toRow - fromRow;
  int dCol = toCol - fromCol;

  switch (type) {
  case PIECE_ROOK:
    // Rook moves horizontally or vertically
    if (dRow != 0 && dCol != 0)
      return false;
    // Check path is clear
    {
      int stepRow = (dRow == 0) ? 0 : (dRow > 0 ? 1 : -1);
      int stepCol = (dCol == 0) ? 0 : (dCol > 0 ? 1 : -1);
      int r = fromRow + stepRow;
      int c = fromCol + stepCol;
      while (r != toRow || c != toCol) {
        if (board[r][c].type != PIECE_NONE)
          return false;
        r += stepRow;
        c += stepCol;
      }
    }
    return true;

  case PIECE_KNIGHT:
    // Knight moves in L-shape
    return (abs(dRow) == 2 && abs(dCol) == 1) ||
           (abs(dRow) == 1 && abs(dCol) == 2);

  case PIECE_BISHOP:
    // Bishop moves diagonally
    if (abs(dRow) != abs(dCol))
      return false;
    // Check path is clear
    {
      int stepRow = (dRow > 0) ? 1 : -1;
      int stepCol = (dCol > 0) ? 1 : -1;
      int r = fromRow + stepRow;
      int c = fromCol + stepCol;
      while (r != toRow || c != toCol) {
        if (board[r][c].type != PIECE_NONE)
          return false;
        r += stepRow;
        c += stepCol;
      }
    }
    return true;

  case PIECE_QUEEN:
    // Queen moves like rook or bishop
    if (dRow == 0 || dCol == 0) {
      // Rook-like move
      int stepRow = (dRow == 0) ? 0 : (dRow > 0 ? 1 : -1);
      int stepCol = (dCol == 0) ? 0 : (dCol > 0 ? 1 : -1);
      int r = fromRow + stepRow;
      int c = fromCol + stepCol;
      while (r != toRow || c != toCol) {
        if (board[r][c].type != PIECE_NONE)
          return false;
        r += stepRow;
        c += stepCol;
      }
      return true;
    } else if (abs(dRow) == abs(dCol)) {
      // Bishop-like move
      int stepRow = (dRow > 0) ? 1 : -1;
      int stepCol = (dCol > 0) ? 1 : -1;
      int r = fromRow + stepRow;
      int c = fromCol + stepCol;
      while (r != toRow || c != toCol) {
        if (board[r][c].type != PIECE_NONE)
          return false;
        r += stepRow;
        c += stepCol;
      }
      return true;
    }
    return false;

  default:
    return false;
  }
}

// Returns: 0 = no disambiguation, 1 = add file, 2 = add rank, 3 = add both
static int GetDisambiguationType(MoveRecord *move) {
  PieceType type = move->pieceType;
  PieceColor color = move->color;

  if (type == PIECE_PAWN || type == PIECE_KING) {
    return 0;
  }

  bool needsFile = false;
  bool needsRank = false;

  for (int row = 0; row < BOARD_SIZE; row++) {
    for (int col = 0; col < BOARD_SIZE; col++) {
      // Skip the piece that made the move
      if (row == move->fromRow && col == move->fromCol)
        continue;

      // Check if this is the same type of piece
      if (board[row][col].type == type && board[row][col].color == color) {
        // Check if this piece can reach the same target square
        if (CanPieceReachSquare(row, col, move->toRow, move->toCol, type)) {
          // Ambiguity exists - determine what disambiguation is needed
          if (col == move->fromCol) {
            // Same file - need rank
            needsRank = true;
          } else {
            // Different file - need file
            needsFile = true;
          }
        }
      }
    }
  }

  if (needsFile && needsRank) {
    return 3; // Need both
  } else if (needsRank) {
    return 2; // Need rank only
  } else if (needsFile) {
    return 1; // Need file only
  }
  return 0;
}

void GenerateMoveNotation(MoveRecord *move) {
  char *notation = move->notation;
  int idx = 0;

  // Castling notation
  if (move->isCastleKingside) {
    strcpy(notation, "O-O");
    idx = 3;
  } else if (move->isCastleQueenside) {
    strcpy(notation, "O-O-O");
    idx = 5;
  } else {
    // Piece symbol (not for pawns)
    char pieceChar = PieceToChar(move->pieceType);
    if (pieceChar != '\0') {
      notation[idx++] = pieceChar;
    }

    // Disambiguation for pieces (or file for pawn captures)
    if (move->pieceType == PIECE_PAWN) {
      if (move->isCapture) {
        notation[idx++] = ColToFile(move->fromCol);
      }
    } else {
      int disambig = GetDisambiguationType(move);
      if (disambig == 3) {
        // Both file and rank needed
        notation[idx++] = ColToFile(move->fromCol);
        notation[idx++] = RowToRank(move->fromRow);
      } else if (disambig == 2) {
        // Rank only
        notation[idx++] = RowToRank(move->fromRow);
      } else if (disambig == 1) {
        // File only
        notation[idx++] = ColToFile(move->fromCol);
      }
    }

    // Capture symbol
    if (move->isCapture) {
      notation[idx++] = 'x';
    }

    // Destination square
    notation[idx++] = ColToFile(move->toCol);
    notation[idx++] = RowToRank(move->toRow);

    // Promotion
    if (move->isPromotion) {
      notation[idx++] = '=';
      notation[idx++] = PieceToChar(move->promotedTo);
    }
  }

  // Check or checkmate
  if (move->givesCheckmate) {
    notation[idx++] = '#';
  } else if (move->givesCheck) {
    notation[idx++] = '+';
  }

  notation[idx] = '\0';
}

//==============================================================================
// MOVE RECORDING
//==============================================================================

void RecordMove(int fromRow, int fromCol, int toRow, int toCol, bool isCapture,
                bool isCastleKingside, bool isCastleQueenside, bool isEnPassant,
                bool isPromotion, PieceType promotedTo) {
  if (moveCount >= MAX_MOVES)
    return;

  MoveRecord *move = &moveHistory[moveCount];

  move->fromRow = fromRow;
  move->fromCol = fromCol;
  move->toRow = toRow;
  move->toCol = toCol;
  move->pieceType = board[fromRow][fromCol].type;
  move->capturedType = board[toRow][toCol].type;
  move->color = board[fromRow][fromCol].color;
  move->isCapture = isCapture;
  move->isCastleKingside = isCastleKingside;
  move->isCastleQueenside = isCastleQueenside;
  move->isEnPassant = isEnPassant;
  move->isPromotion = isPromotion;
  move->promotedTo = promotedTo;
  move->givesCheck = false;
  move->givesCheckmate = false;

  // Generate notation (will be updated after check status is known)
  GenerateMoveNotation(move);

  moveCount++;
}

void UpdateLastMoveStatus(bool givesCheck, bool givesCheckmate) {
  if (moveCount == 0)
    return;

  MoveRecord *move = &moveHistory[moveCount - 1];
  move->givesCheck = givesCheck;
  move->givesCheckmate = givesCheckmate;

  // Regenerate notation with check/checkmate symbols
  GenerateMoveNotation(move);
}
