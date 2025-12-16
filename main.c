/**
 * Chess Game - A complete chess implementation using Raylib
 *
 * Features:
 * - Full chess rules including castling, en passant, and pawn promotion
 * - Click-to-move and drag-and-drop piece movement
 * - Check, checkmate, and stalemate detection
 * - Visual highlighting for valid moves and check state
 * - Chess clock with multiple time control modes
 */

#include "board.h"
#include "check.h"
#include "clock.h"
#include "menu.h"
#include "moves.h"
#include "raylib.h"
#include "types.h"
#include "ui.h"

int main(void) {
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Chess");
  SetTargetFPS(60);
  SetExitKey(0); // Disable ESC closing the window

  LoadPiecesTexture();
  InitFloatingPieces();
  InitClockConfig();
  InitBoard();

  while (!WindowShouldClose()) {
    // ESC returns to menu (does nothing on title screen)
    // Clock setup has its own ESC handling
    if (IsKeyPressed(KEY_ESCAPE) && currentScreen != SCREEN_TITLE &&
        currentScreen != SCREEN_CLOCK_SETUP) {
      StopClock();
      currentScreen = SCREEN_TITLE;
      continue;
    }

    // Update and handle input based on current screen
    switch (currentScreen) {
    case SCREEN_TITLE:
      UpdateFloatingPieces();
      HandleTitleInput();
      break;

    case SCREEN_CLOCK_SETUP:
      UpdateFloatingPieces();
      HandleClockSetupInput();
      break;

    case SCREEN_OPTIONS:
      UpdateFloatingPieces();
      HandleOptionsInput();
      break;

    case SCREEN_GAME:
      // Update clock and check for timeout
      if (gameState == GAME_PLAYING || gameState == GAME_CHECK) {
        UpdateClock(currentTurn);
        PieceColor flagged = CheckTimeout();
        if (flagged != COLOR_NONE) {
          gameState = GAME_TIMEOUT;
        }
      }

      if (gameState == GAME_PROMOTING) {
        HandlePromotion();
      } else if (gameState == GAME_CHECKMATE || gameState == GAME_STALEMATE ||
                 gameState == GAME_TIMEOUT) {
        if (IsKeyPressed(KEY_R)) {
          InitBoard();
          InitClock();
          StartClock();
        }
      } else {
        HandleInput();
      }
      break;
    }

    // Render based on current screen
    BeginDrawing();
    ClearBackground(COLOR_BACKGROUND);

    switch (currentScreen) {
    case SCREEN_TITLE:
      DrawTitleScreen();
      break;

    case SCREEN_CLOCK_SETUP:
      DrawTitleScreen();
      DrawClockSetupScreen();
      break;

    case SCREEN_OPTIONS:
      DrawTitleScreen();
      DrawOptionsScreen();
      break;

    case SCREEN_GAME:
      DrawBoard();
      DrawValidMoves();
      DrawPieces();
      DrawUI();
      DrawClocks();
      DrawMoveHistory();

      if (gameState == GAME_PROMOTING) {
        DrawPromotionUI();
      } else if (gameState == GAME_CHECKMATE || gameState == GAME_STALEMATE ||
                 gameState == GAME_TIMEOUT) {
        DrawGameOverScreen();
      }
      break;
    }

    EndDrawing();
  }

  UnloadPiecesTexture();
  CloseWindow();
  return 0;
}
