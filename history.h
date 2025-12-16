/**
 * Chess Game - Move History
 * Recording and displaying moves in algebraic notation.
 */

#ifndef HISTORY_H
#define HISTORY_H

#include "types.h"

//==============================================================================
// MOVE RECORD STRUCTURE
//==============================================================================

typedef struct {
  int fromRow, fromCol;
  int toRow, toCol;
  PieceType pieceType;
  PieceType capturedType;
  PieceColor color;
  bool isCapture;
  bool isCastleKingside;
  bool isCastleQueenside;
  bool isEnPassant;
  bool isPromotion;
  PieceType promotedTo;
  bool givesCheck;
  bool givesCheckmate;
  char notation[MOVE_NOTATION_LEN];
} MoveRecord;

//==============================================================================
// GLOBAL HISTORY STATE
//==============================================================================

extern MoveRecord moveHistory[MAX_MOVES];
extern int moveCount;
extern int historyScrollOffset;

//==============================================================================
// HISTORY FUNCTIONS
//==============================================================================

/**
 * Clear move history (call when starting a new game).
 */
void InitMoveHistory(void);

/**
 * Record a move after it's executed.
 * pieceType and color must be passed explicitly since the from-square is
 * already empty.
 */
void RecordMove(int fromRow, int fromCol, int toRow, int toCol,
                PieceType pieceType, PieceColor color, bool isCapture,
                bool isCastleKingside, bool isCastleQueenside, bool isEnPassant,
                bool isPromotion, PieceType promotedTo);

/**
 * Update the last move's check/checkmate status.
 * Call after UpdateGameState().
 */
void UpdateLastMoveStatus(bool givesCheck, bool givesCheckmate);

/**
 * Generate algebraic notation for a move.
 */
void GenerateMoveNotation(MoveRecord *move);

/**
 * Get total number of moves recorded.
 */
int GetMoveCount(void);

#endif // HISTORY_H
