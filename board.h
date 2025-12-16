/**
 * Chess Game - Board Management
 * Board state and basic query operations.
 */

#ifndef BOARD_H
#define BOARD_H

#include "types.h"

//==============================================================================
// GLOBAL BOARD STATE (defined in board.c)
//==============================================================================

extern Piece board[BOARD_SIZE][BOARD_SIZE];
extern PieceColor currentTurn;
extern Position selectedPos;
extern GameState gameState;
extern ScreenState currentScreen;
extern Position enPassantTarget;
extern Position enPassantPawn;
extern Position promotionPos;
extern bool isDragging;
extern Position dragStartPos;
extern Vector2 dragOffset;

//==============================================================================
// BOARD FUNCTIONS
//==============================================================================

/**
 * Initialize the board with pieces in starting positions.
 * Resets all game state including turn, en passant, and selection.
 */
void InitBoard(void);

/**
 * Check if a position is within board bounds.
 */
bool IsValidPosition(int row, int col);

/**
 * Check if a square is empty.
 */
bool IsEmpty(int row, int col);

/**
 * Check if a square contains an enemy piece.
 */
bool IsEnemy(int row, int col, PieceColor color);

/**
 * Check if a square contains an allied piece.
 */
bool IsAlly(int row, int col, PieceColor color);

/**
 * Find the position of a specific color's king.
 */
Position FindKing(PieceColor color);

#endif // BOARD_H
