/**
 * Chess Game - Multiplayer Module Header
 * Game state synchronization for P2P multiplayer.
 */

#ifndef MULTIPLAYER_H
#define MULTIPLAYER_H

#include "types.h"
#include <stdbool.h>

//==============================================================================
// MULTIPLAYER TYPES
//==============================================================================

typedef enum { MP_ROLE_NONE = 0, MP_ROLE_HOST, MP_ROLE_GUEST } MultiplayerRole;

//==============================================================================
// MULTIPLAYER STATE (defined in multiplayer.c)
//==============================================================================

extern MultiplayerRole multiplayerRole;
extern PieceColor localPlayerColor;
extern bool isMultiplayerGame;

//==============================================================================
// MULTIPLAYER FUNCTIONS
//==============================================================================

/**
 * Initialize multiplayer state.
 */
void InitMultiplayer(void);

/**
 * Start a multiplayer game (called when connection is established).
 */
void StartMultiplayerGame(void);

/**
 * Reset multiplayer state.
 */
void ResetMultiplayer(void);

/**
 * Update multiplayer state (called each frame during game).
 */
void UpdateMultiplayer(void);

/**
 * Check if it's the local player's turn.
 */
bool IsLocalPlayerTurn(void);

/**
 * Handle a move made locally in multiplayer.
 * Sends the move to the remote player.
 */
void HandleLocalMove(int fromRow, int fromCol, int toRow, int toCol,
                     int promotionPiece);

/**
 * Process a move received from the remote player.
 */
void ProcessRemoteMove(int fromRow, int fromCol, int toRow, int toCol,
                       int promotionPiece);

#endif // MULTIPLAYER_H
