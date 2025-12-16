/**
 * Chess Game - Check Detection
 * Check, checkmate, and stalemate detection.
 */

#ifndef CHECK_H
#define CHECK_H

#include "types.h"

/**
 * Determine if a square is under attack by any piece of the specified color.
 */
bool IsSquareAttacked(int row, int col, PieceColor byColor);

/**
 * Check if the specified color's king is currently in check.
 */
bool IsInCheck(PieceColor color);

/**
 * Simulate a move and check if it would leave the king in check.
 */
bool WouldBeInCheck(int fromRow, int fromCol, int toRow, int toCol,
                    PieceColor color);

/**
 * Check if the specified color has any legal moves available.
 */
bool HasLegalMoves(PieceColor color);

/**
 * Update game state based on current board position
 * (check/checkmate/stalemate).
 */
void UpdateGameState(void);

#endif // CHECK_H
