/**
 * Chess Game - Move History
 * Recording and displaying moves in algebraic notation.
 */

#include "history.h"
#include "board.h"
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

static bool NeedsDisambiguation(MoveRecord *move) {
  // Check if another piece of the same type can move to the same square
  PieceType type = move->pieceType;
  PieceColor color = move->color;

  if (type == PIECE_PAWN || type == PIECE_KING) {
    return false;
  }

  int count = 0;
  int sameFile = 0;
  int sameRank = 0;

  for (int row = 0; row < BOARD_SIZE; row++) {
    for (int col = 0; col < BOARD_SIZE; col++) {
      if (row == move->fromRow && col == move->fromCol)
        continue;
      if (board[row][col].type == type && board[row][col].color == color) {
        // This is another piece of the same type
        // For simplicity, we check if it could potentially reach the target
        // In a fully correct implementation, we'd need to verify the move is
        // legal
        count++;
        if (col == move->fromCol)
          sameFile++;
        if (row == move->fromRow)
          sameRank++;
      }
    }
  }

  // For pawns capturing, we always show the file
  if (type == PIECE_PAWN && move->isCapture) {
    return true;
  }

  return count > 0;
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
    } else if (NeedsDisambiguation(move)) {
      // Add file, or rank if same file, or both if needed
      notation[idx++] = ColToFile(move->fromCol);
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
