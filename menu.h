/**
 * Chess Game - Menu Screens
 * Title screen and options menu.
 */

#ifndef MENU_H
#define MENU_H

#include "types.h"

//==============================================================================
// ANIMATION CONFIGURATION
//==============================================================================

#define MAX_FLOATING_PIECES 12

//==============================================================================
// MENU FUNCTIONS
//==============================================================================

/**
 * Initialize floating pieces for title screen animation.
 */
void InitFloatingPieces(void);

/**
 * Update floating piece positions and rotations.
 */
void UpdateFloatingPieces(void);

/**
 * Draw the title screen with animated background.
 */
void DrawTitleScreen(void);

/**
 * Handle title screen input.
 */
void HandleTitleInput(void);

/**
 * Draw the options overlay.
 */
void DrawOptionsScreen(void);

/**
 * Handle options screen input.
 */
void HandleOptionsInput(void);

/**
 * Draw a menu button and return true if clicked.
 */
bool DrawMenuButton(int x, int y, int width, int height, const char *text);

/**
 * Draw the clock setup screen.
 */
void DrawClockSetupScreen(void);

/**
 * Handle clock setup screen input.
 */
void HandleClockSetupInput(void);

/**
 * Draw the multiplayer menu screen.
 */
void DrawMultiplayerScreen(void);

/**
 * Handle multiplayer menu input.
 */
void HandleMultiplayerInput(void);

/**
 * Draw the host game screen.
 */
void DrawHostScreen(void);

/**
 * Handle host screen input.
 */
void HandleHostInput(void);

/**
 * Draw the join game screen.
 */
void DrawJoinScreen(void);

/**
 * Handle join screen input.
 */
void HandleJoinInput(void);

#endif // MENU_H
