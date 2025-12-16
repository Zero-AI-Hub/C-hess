/**
 * Chess Game - Multiplayer Module
 * Game state synchronization for P2P multiplayer.
 */

#include "multiplayer.h"
#include "board.h"
#include "check.h"
#include "clock.h"
#include "history.h"
#include "moves.h"
#include "network.h"
#include <stdio.h>

//==============================================================================
// MULTIPLAYER STATE
//==============================================================================

MultiplayerRole multiplayerRole = MP_ROLE_NONE;
PieceColor localPlayerColor = COLOR_NONE;
bool isMultiplayerGame = false;

// Flag to track if we're processing a remote move
static bool processingRemoteMove = false;

//==============================================================================
// NETWORK CALLBACK
//==============================================================================

static void OnRemoteMoveReceived(int fromRow, int fromCol, int toRow, int toCol,
                                 int promotionPiece) {
  ProcessRemoteMove(fromRow, fromCol, toRow, toCol, promotionPiece);
}

//==============================================================================
// PUBLIC FUNCTIONS
//==============================================================================

void InitMultiplayer(void) {
  multiplayerRole = MP_ROLE_NONE;
  localPlayerColor = COLOR_NONE;
  isMultiplayerGame = false;
  processingRemoteMove = false;

  InitNetwork();
  SetMoveCallback(OnRemoteMoveReceived);
}

void StartMultiplayerGame(void) {
  isMultiplayerGame = true;

  // Host plays white, guest plays black
  if (multiplayerRole == MP_ROLE_HOST) {
    localPlayerColor = COLOR_WHITE;
    printf("[Multiplayer] Starting as HOST (White)\n");
  } else if (multiplayerRole == MP_ROLE_GUEST) {
    localPlayerColor = COLOR_BLACK;
    printf("[Multiplayer] Starting as GUEST (Black)\n");
  }

  // Initialize the game
  InitBoard();
  InitClock();
  StartClock();
}

void ResetMultiplayer(void) {
  multiplayerRole = MP_ROLE_NONE;
  localPlayerColor = COLOR_NONE;
  isMultiplayerGame = false;
  processingRemoteMove = false;
  DisconnectNetwork();
}

void UpdateMultiplayer(void) {
  // Poll network for incoming messages
  PollNetwork();
}

bool IsLocalPlayerTurn(void) {
  if (!isMultiplayerGame) {
    return true; // Local game - always your turn
  }
  return currentTurn == localPlayerColor;
}

void HandleLocalMove(int fromRow, int fromCol, int toRow, int toCol,
                     int promotionPiece) {
  if (!isMultiplayerGame) {
    return;
  }

  // Only send if it's our turn and we're not processing a remote move
  if (!processingRemoteMove && IsLocalPlayerTurn()) {
    NetworkSendMove(fromRow, fromCol, toRow, toCol, promotionPiece);
  }
}

void ProcessRemoteMove(int fromRow, int fromCol, int toRow, int toCol,
                       int promotionPiece) {
  if (!isMultiplayerGame) {
    return;
  }

  // Only process if it's the opponent's turn
  if (IsLocalPlayerTurn()) {
    printf("[Multiplayer] Ignoring remote move - it's our turn\n");
    return;
  }

  printf("[Multiplayer] Processing remote move: %d,%d -> %d,%d\n", fromRow,
         fromCol, toRow, toCol);

  // Set flag to prevent sending the move back
  processingRemoteMove = true;

  // Select the piece
  selectedPos = (Position){fromRow, fromCol};
  ClearValidMoves();
  CalculateValidMoves(fromRow, fromCol);

  // Validate and execute the move
  if (IsValidMove(toRow, toCol)) {
    // Execute the move
    MovePiece(toRow, toCol);

    // Handle promotion if needed
    if (gameState == GAME_PROMOTING && promotionPiece > 0) {
      // Apply the promotion
      board[toRow][toCol].type = (PieceType)promotionPiece;

      // Record the move with promotion
      RecordMove(fromRow, fromCol, toRow, toCol, PIECE_PAWN,
                 OPPONENT_COLOR(currentTurn), true, false, false, false, true,
                 (PieceType)promotionPiece);

      // Complete the turn
      SwitchClock(OPPONENT_COLOR(currentTurn));
      gameState = GAME_PLAYING;
      UpdateGameState();
      UpdateLastMoveStatus(gameState == GAME_CHECK ||
                               gameState == GAME_CHECKMATE,
                           gameState == GAME_CHECKMATE);
    }
  } else {
    printf("[Multiplayer] Remote move was invalid!\n");
  }

  processingRemoteMove = false;
  selectedPos = INVALID_POS;
  ClearValidMoves();
}
