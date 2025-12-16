/**
 * Chess Game - UI and Drawing
 * Drawing functions and game input handling.
 */

#ifndef UI_H
#define UI_H

#include "types.h"

//==============================================================================
// GLOBAL TEXTURE (defined in ui.c)
//==============================================================================

extern Texture2D piecesTexture;

//==============================================================================
// DRAWING FUNCTIONS
//==============================================================================

/**
 * Load the chess pieces sprite sheet.
 */
void LoadPiecesTexture(void);

/**
 * Unload the chess pieces texture.
 */
void UnloadPiecesTexture(void);

/**
 * Draw the chessboard with square highlighting.
 */
void DrawBoard(void);

/**
 * Draw valid move indicators (circles for moves, squares for captures).
 */
void DrawValidMoves(void);

/**
 * Draw all pieces on the board (including dragged piece).
 */
void DrawPieces(void);

/**
 * Draw the turn indicator and game state text.
 */
void DrawUI(void);

/**
 * Draw the pawn promotion selection UI.
 */
void DrawPromotionUI(void);

/**
 * Draw the game over (checkmate/stalemate) overlay.
 */
void DrawGameOverScreen(void);

/**
 * Draw the move history panel on the right side of the board.
 */
void DrawMoveHistory(void);

/**
 * Draw the chess clocks for both players.
 */
void DrawClocks(void);

//==============================================================================
// INPUT HANDLING
//==============================================================================

/**
 * Handle mouse input for piece selection and movement.
 */
void HandleInput(void);

/**
 * Handle promotion piece selection.
 */
void HandlePromotion(void);

//==============================================================================
// SPRITE HELPER
//==============================================================================

/**
 * Get the sprite rectangle for a given piece type and color.
 */
Rectangle GetSpriteRect(PieceType type, PieceColor color);

/**
 * Get the promotion panel layout.
 */
PanelLayout GetPromotionPanelLayout(void);

#endif // UI_H
