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
                                PieceType type, PieceColor color) {
  // Check if a piece at (fromRow, fromCol) can legally reach (toRow, toCol)
  // This must check: movement pattern, path clearance, target not friendly,
  // and that the move doesn't leave the king in check

  int dRow = toRow - fromRow;
  int dCol = toCol - fromCol;

  // Target square cannot be occupied by a friendly piece
  if (board[toRow][toCol].type != PIECE_NONE &&
      board[toRow][toCol].color == color) {
    return false;
  }

  bool patternValid = false;

  switch (type) {
  case PIECE_ROOK:
    // Rook moves horizontally or vertically
    if (dRow != 0 && dCol != 0)
      return false;
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
    patternValid = true;
    break;

  case PIECE_KNIGHT:
    patternValid = (abs(dRow) == 2 && abs(dCol) == 1) ||
                   (abs(dRow) == 1 && abs(dCol) == 2);
    break;

  case PIECE_BISHOP:
    if (abs(dRow) != abs(dCol))
      return false;
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
    patternValid = true;
    break;

  case PIECE_QUEEN:
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
      patternValid = true;
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
      patternValid = true;
    }
    break;

  default:
    return false;
  }

  if (!patternValid)
    return false;

  // Check if this move would leave the king in check
  // Temporarily make the move
  Piece movingPiece = board[fromRow][fromCol];
  Piece capturedPiece = board[toRow][toCol];

  board[toRow][toCol] = movingPiece;
  board[fromRow][fromCol] = (Piece){PIECE_NONE, COLOR_NONE, false};

  // Find the king and check if it's in check
  bool inCheck = false;
  for (int row = 0; row < BOARD_SIZE && !inCheck; row++) {
    for (int col = 0; col < BOARD_SIZE && !inCheck; col++) {
      if (board[row][col].type == PIECE_KING &&
          board[row][col].color == color) {
        // Check if this king position is attacked by any enemy piece
        PieceColor enemy = (color == COLOR_WHITE) ? COLOR_BLACK : COLOR_WHITE;

        // Check pawn attacks
        int pawnDir = (enemy == COLOR_WHITE) ? 1 : -1;
        for (int dc = -1; dc <= 1; dc += 2) {
          int pr = row + pawnDir;
          int pc = col + dc;
          if (pr >= 0 && pr < BOARD_SIZE && pc >= 0 && pc < BOARD_SIZE &&
              board[pr][pc].type == PIECE_PAWN &&
              board[pr][pc].color == enemy) {
            inCheck = true;
          }
        }

        // Check knight attacks
        for (int i = 0; i < 8 && !inCheck; i++) {
          int nr = row + KNIGHT_MOVES[i][0];
          int nc = col + KNIGHT_MOVES[i][1];
          if (nr >= 0 && nr < BOARD_SIZE && nc >= 0 && nc < BOARD_SIZE &&
              board[nr][nc].type == PIECE_KNIGHT &&
              board[nr][nc].color == enemy) {
            inCheck = true;
          }
        }

        // Check rook/queen attacks (straight lines)
        for (int d = 0; d < 4 && !inCheck; d++) {
          for (int i = 1; i < BOARD_SIZE; i++) {
            int tr = row + i * ROOK_DIRECTIONS[d][0];
            int tc = col + i * ROOK_DIRECTIONS[d][1];
            if (tr < 0 || tr >= BOARD_SIZE || tc < 0 || tc >= BOARD_SIZE)
              break;
            if (board[tr][tc].type != PIECE_NONE) {
              if (board[tr][tc].color == enemy &&
                  (board[tr][tc].type == PIECE_ROOK ||
                   board[tr][tc].type == PIECE_QUEEN)) {
                inCheck = true;
              }
              break;
            }
          }
        }

        // Check bishop/queen attacks (diagonals)
        for (int d = 0; d < 4 && !inCheck; d++) {
          for (int i = 1; i < BOARD_SIZE; i++) {
            int tr = row + i * BISHOP_DIRECTIONS[d][0];
            int tc = col + i * BISHOP_DIRECTIONS[d][1];
            if (tr < 0 || tr >= BOARD_SIZE || tc < 0 || tc >= BOARD_SIZE)
              break;
            if (board[tr][tc].type != PIECE_NONE) {
              if (board[tr][tc].color == enemy &&
                  (board[tr][tc].type == PIECE_BISHOP ||
                   board[tr][tc].type == PIECE_QUEEN)) {
                inCheck = true;
              }
              break;
            }
          }
        }

        // Check king attacks (adjacent squares)
        for (int dr = -1; dr <= 1 && !inCheck; dr++) {
          for (int dc = -1; dc <= 1 && !inCheck; dc++) {
            if (dr == 0 && dc == 0)
              continue;
            int kr = row + dr;
            int kc = col + dc;
            if (kr >= 0 && kr < BOARD_SIZE && kc >= 0 && kc < BOARD_SIZE &&
                board[kr][kc].type == PIECE_KING &&
                board[kr][kc].color == enemy) {
              inCheck = true;
            }
          }
        }
      }
    }
  }

  // Restore the board
  board[fromRow][fromCol] = movingPiece;
  board[toRow][toCol] = capturedPiece;

  return !inCheck;
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
        if (CanPieceReachSquare(row, col, move->toRow, move->toCol, type,
                                color)) {
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

void RecordMove(int fromRow, int fromCol, int toRow, int toCol,
                PieceType pieceType, PieceColor color, bool isCapture,
                bool isCastleKingside, bool isCastleQueenside, bool isEnPassant,
                bool isPromotion, PieceType promotedTo) {
  if (moveCount >= MAX_MOVES)
    return;

  MoveRecord *move = &moveHistory[moveCount];

  move->fromRow = fromRow;
  move->fromCol = fromCol;
  move->toRow = toRow;
  move->toCol = toCol;
  move->pieceType = pieceType;
  move->capturedType = PIECE_NONE; // Could be passed as param if needed
  move->color = color;
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
