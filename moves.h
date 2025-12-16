/**
 * Chess Game - Move Validation
 * Move calculation and execution.
 */

#ifndef MOVES_H
#define MOVES_H

#include "types.h"

//==============================================================================
// VALID MOVES ARRAY (defined in moves.c)
//==============================================================================

extern bool validMoves[BOARD_SIZE][BOARD_SIZE];

//==============================================================================
// MOVE FUNCTIONS
//==============================================================================

/**
 * Clear all valid moves.
 */
void ClearValidMoves(void);

/**
 * Check if a move to (row, col) is valid.
 */
bool IsValidMove(int row, int col);

/**
 * Calculate all valid moves for the piece at (row, col).
 */
void CalculateValidMoves(int row, int col);

/**
 * Add a move to validMoves if it's legal (doesn't leave king in check).
 */
void AddMoveIfValid(int row, int col, PieceColor color);

/**
 * Execute a move from selectedPos to (toRow, toCol).
 * Handles castling, en passant, and promotion.
 */
void MovePiece(int toRow, int toCol);

#endif // MOVES_H
