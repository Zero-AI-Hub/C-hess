/**
 * Chess Game - UI and Drawing
 * Drawing functions and game input handling.
 */

#include "ui.h"
#include "board.h"
#include "check.h"
#include "history.h"
#include "moves.h"
#include <stdio.h>

//==============================================================================
// GLOBAL TEXTURE
//==============================================================================

Texture2D piecesTexture;

//==============================================================================
// TEXTURE LOADING
//==============================================================================

void LoadPiecesTexture(void) { piecesTexture = LoadTexture("ChessPieces.png"); }

void UnloadPiecesTexture(void) { UnloadTexture(piecesTexture); }

//==============================================================================
// SPRITE HELPERS
//==============================================================================

Rectangle GetSpriteRect(PieceType type, PieceColor color) {
  // Map piece type enum to sprite column index
  static const int SPRITE_COL[7] = {0, 1, 0, 4, 3, 2, 5};
  int col = SPRITE_COL[type];
  int row = (color == COLOR_WHITE) ? 1 : 0;
  return (Rectangle){col * SPRITE_SIZE, row * SPRITE_SIZE, SPRITE_SIZE,
                     SPRITE_SIZE};
}

PanelLayout GetPromotionPanelLayout(void) {
  int panelWidth = PANEL_PADDING * 2 + 4 * TILE_SIZE + 3 * BUTTON_SPACING;
  int panelHeight = TILE_SIZE + 70;
  return (PanelLayout){(WINDOW_WIDTH - panelWidth) / 2,
                       (WINDOW_HEIGHT - panelHeight) / 2, panelWidth,
                       panelHeight};
}

//==============================================================================
// BOARD DRAWING
//==============================================================================

void DrawBoard(void) {
  for (int row = 0; row < BOARD_SIZE; row++) {
    for (int col = 0; col < BOARD_SIZE; col++) {
      int x = BOARD_OFFSET_X + col * TILE_SIZE;
      int y = BOARD_OFFSET_Y + row * TILE_SIZE;

      // Alternating square colors
      Color squareColor =
          ((row + col) % 2 == 0) ? COLOR_LIGHT_SQUARE : COLOR_DARK_SQUARE;
      DrawRectangle(x, y, TILE_SIZE, TILE_SIZE, squareColor);

      // Highlight selected square
      if (row == selectedPos.row && col == selectedPos.col) {
        DrawRectangle(x, y, TILE_SIZE, TILE_SIZE, COLOR_SELECTED);
      }

      // Highlight king in check
      if ((gameState == GAME_CHECK || gameState == GAME_CHECKMATE) &&
          board[row][col].type == PIECE_KING &&
          board[row][col].color == currentTurn) {
        DrawRectangle(x, y, TILE_SIZE, TILE_SIZE, COLOR_CHECK_HIGHLIGHT);
      }
    }
  }

  // Board border
  DrawRectangleLines(BOARD_OFFSET_X - 2, BOARD_OFFSET_Y - 2,
                     BOARD_SIZE * TILE_SIZE + 4, BOARD_SIZE * TILE_SIZE + 4,
                     WHITE);
}

void DrawValidMoves(void) {
  for (int row = 0; row < BOARD_SIZE; row++) {
    for (int col = 0; col < BOARD_SIZE; col++) {
      if (!IsValidMove(row, col))
        continue;

      int x = BOARD_OFFSET_X + col * TILE_SIZE;
      int y = BOARD_OFFSET_Y + row * TILE_SIZE;

      // Capture moves shown as red overlay, regular moves as green circle
      bool isCapture =
          board[row][col].type != PIECE_NONE ||
          (row == enPassantTarget.row && col == enPassantTarget.col);

      if (isCapture) {
        DrawRectangle(x, y, TILE_SIZE, TILE_SIZE, COLOR_CAPTURE);
      } else {
        DrawCircle(x + TILE_SIZE / 2, y + TILE_SIZE / 2, VALID_MOVE_RADIUS,
                   COLOR_VALID_MOVE);
      }
    }
  }
}

//==============================================================================
// PIECE DRAWING
//==============================================================================

void DrawPieces(void) {
  // Draw all pieces on the board (skip dragged piece)
  for (int row = 0; row < BOARD_SIZE; row++) {
    for (int col = 0; col < BOARD_SIZE; col++) {
      if (board[row][col].type == PIECE_NONE)
        continue;
      if (isDragging && row == dragStartPos.row && col == dragStartPos.col)
        continue;

      Rectangle src =
          GetSpriteRect(board[row][col].type, board[row][col].color);
      int x = BOARD_OFFSET_X + col * TILE_SIZE + (TILE_SIZE - SPRITE_SIZE) / 2;
      int y = BOARD_OFFSET_Y + row * TILE_SIZE + (TILE_SIZE - SPRITE_SIZE) / 2;
      Rectangle dest = {x, y, SPRITE_SIZE, SPRITE_SIZE};

      DrawTexturePro(piecesTexture, src, dest, (Vector2){0, 0}, 0, WHITE);
    }
  }

  // Draw dragged piece at mouse position
  if (isDragging && dragStartPos.row != -1) {
    Piece draggedPiece = board[dragStartPos.row][dragStartPos.col];
    if (draggedPiece.type != PIECE_NONE) {
      Vector2 mouse = GetMousePosition();
      Rectangle src = GetSpriteRect(draggedPiece.type, draggedPiece.color);
      Rectangle dest = {mouse.x - SPRITE_SIZE / 2 - dragOffset.x,
                        mouse.y - SPRITE_SIZE / 2 - dragOffset.y, SPRITE_SIZE,
                        SPRITE_SIZE};
      DrawTexturePro(piecesTexture, src, dest, (Vector2){0, 0}, 0, WHITE);
    }
  }
}

//==============================================================================
// UI ELEMENTS
//==============================================================================

void DrawUI(void) {
  int y = WINDOW_HEIGHT - 50;

  const char *turnText =
      (currentTurn == COLOR_WHITE) ? "White's Turn" : "Black's Turn";
  DrawText(turnText, BOARD_OFFSET_X, y, FONT_SIZE_MEDIUM, WHITE);

  // Status text based on game state
  const char *stateText = "";
  Color stateColor = WHITE;

  switch (gameState) {
  case GAME_CHECK:
    stateText = " - CHECK!";
    stateColor = YELLOW;
    break;
  case GAME_CHECKMATE:
    stateText = (currentTurn == COLOR_WHITE) ? " - CHECKMATE! Black wins!"
                                             : " - CHECKMATE! White wins!";
    stateColor = RED;
    break;
  case GAME_STALEMATE:
    stateText = " - STALEMATE! Draw!";
    stateColor = GRAY;
    break;
  default:
    break;
  }

  DrawText(stateText, BOARD_OFFSET_X + MeasureText(turnText, FONT_SIZE_MEDIUM),
           y, FONT_SIZE_MEDIUM, stateColor);

  if (gameState == GAME_CHECKMATE || gameState == GAME_STALEMATE) {
    DrawText("Press R to restart",
             BOARD_OFFSET_X + BOARD_SIZE * TILE_SIZE - 180, y, FONT_SIZE_SMALL,
             GRAY);
  }
}

void DrawMoveHistory(void) {
  // Panel position and dimensions
  int panelX = BOARD_OFFSET_X + BOARD_SIZE * TILE_SIZE + 20;
  int panelY = BOARD_OFFSET_Y;
  int panelWidth = HISTORY_PANEL_WIDTH - 30;
  int panelHeight = BOARD_SIZE * TILE_SIZE;

  // Draw panel background
  DrawRectangle(panelX, panelY, panelWidth, panelHeight, COLOR_PANEL_BG);
  DrawRectangleLinesEx((Rectangle){panelX, panelY, panelWidth, panelHeight}, 2,
                       WHITE);

  // Draw title
  const char *title = "Move History";
  int titleWidth = MeasureText(title, FONT_SIZE_SMALL);
  DrawText(title, panelX + (panelWidth - titleWidth) / 2, panelY + 10,
           FONT_SIZE_SMALL, WHITE);

  // Draw moves in PGN format
  int lineHeight = 22;
  int startY = panelY + 40;
  int maxVisibleLines = (panelHeight - 50) / lineHeight;
  int totalMoveCount = GetMoveCount();

  // Calculate number of full moves (pairs)
  int fullMoves = (totalMoveCount + 1) / 2;

  // Auto-scroll to show latest moves
  if (fullMoves > maxVisibleLines) {
    historyScrollOffset = fullMoves - maxVisibleLines;
  } else {
    historyScrollOffset = 0;
  }

  // Draw visible moves
  int visibleLine = 0;
  for (int i = historyScrollOffset;
       i < fullMoves && visibleLine < maxVisibleLines; i++) {
    int y = startY + visibleLine * lineHeight;
    char lineBuffer[64];

    // Move number
    int moveNum = i + 1;

    // White's move (even indices: 0, 2, 4, ...)
    int whiteIdx = i * 2;
    const char *whiteMove =
        (whiteIdx < totalMoveCount) ? moveHistory[whiteIdx].notation : "";

    // Black's move (odd indices: 1, 3, 5, ...)
    int blackIdx = i * 2 + 1;
    const char *blackMove =
        (blackIdx < totalMoveCount) ? moveHistory[blackIdx].notation : "";

    // Format: "1. e4     e5" or "1. e4" if black hasn't moved
    // Using %-8s for fixed width to improve spacing between moves
    if (blackIdx < totalMoveCount) {
      snprintf(lineBuffer, sizeof(lineBuffer), "%d. %-8s %s", moveNum,
               whiteMove, blackMove);
    } else {
      snprintf(lineBuffer, sizeof(lineBuffer), "%d. %s", moveNum, whiteMove);
    }

    // Draw with alternating background for readability
    if (visibleLine % 2 == 0) {
      DrawRectangle(panelX + 5, y - 2, panelWidth - 10, lineHeight,
                    (Color){50, 50, 50, 255});
    }

    DrawText(lineBuffer, panelX + 10, y, FONT_SIZE_SMALL, LIGHTGRAY);
    visibleLine++;
  }

  // Draw scroll indicator if there are more moves
  if (fullMoves > maxVisibleLines) {
    int indicatorY = panelY + panelHeight - 20;
    char scrollText[32];
    snprintf(scrollText, sizeof(scrollText), "... %d moves total",
             totalMoveCount);
    int scrollWidth = MeasureText(scrollText, FONT_SIZE_SMALL - 4);
    DrawText(scrollText, panelX + (panelWidth - scrollWidth) / 2, indicatorY,
             FONT_SIZE_SMALL - 4, GRAY);
  }
}

void DrawPromotionUI(void) {
  DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, COLOR_OVERLAY_DARK);

  PanelLayout panel = GetPromotionPanelLayout();
  DrawRectangle(panel.x, panel.y, panel.width, panel.height, COLOR_PANEL_BG);
  DrawRectangleLinesEx((Rectangle){panel.x, panel.y, panel.width, panel.height},
                       2, WHITE);

  DrawText("Choose promotion:", panel.x + PANEL_PADDING, panel.y + 12,
           FONT_SIZE_SMALL, WHITE);

  PieceType options[] = {PIECE_QUEEN, PIECE_ROOK, PIECE_BISHOP, PIECE_KNIGHT};
  PieceColor color = board[promotionPos.row][promotionPos.col].color;

  for (int i = 0; i < 4; i++) {
    int x = panel.x + PANEL_PADDING + i * (TILE_SIZE + BUTTON_SPACING);
    int y = panel.y + 45;

    DrawRectangle(x, y, TILE_SIZE, TILE_SIZE, COLOR_BUTTON_BG);
    DrawRectangleLines(x, y, TILE_SIZE, TILE_SIZE, WHITE);

    Rectangle src = GetSpriteRect(options[i], color);
    Rectangle dest = {x + (TILE_SIZE - SPRITE_SIZE) / 2,
                      y + (TILE_SIZE - SPRITE_SIZE) / 2, SPRITE_SIZE,
                      SPRITE_SIZE};
    DrawTexturePro(piecesTexture, src, dest, (Vector2){0, 0}, 0, WHITE);
  }
}

void DrawGameOverScreen(void) {
  DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, COLOR_OVERLAY_DARKER);

  int panelWidth = 400;
  int panelHeight = 200;
  int panelX = (WINDOW_WIDTH - panelWidth) / 2;
  int panelY = (WINDOW_HEIGHT - panelHeight) / 2;

  DrawRectangle(panelX, panelY, panelWidth, panelHeight, COLOR_BACKGROUND);
  DrawRectangleLinesEx((Rectangle){panelX, panelY, panelWidth, panelHeight}, 3,
                       WHITE);

  const char *titleText =
      (gameState == GAME_CHECKMATE) ? "CHECKMATE!" : "STALEMATE!";
  const char *subtitleText =
      (gameState == GAME_CHECKMATE)
          ? ((currentTurn == COLOR_WHITE) ? "Black Wins!" : "White Wins!")
          : "It's a Draw!";
  Color titleColor = (gameState == GAME_CHECKMATE) ? RED : GRAY;

  int titleWidth = MeasureText(titleText, FONT_SIZE_TITLE);
  DrawText(titleText, panelX + (panelWidth - titleWidth) / 2, panelY + 30,
           FONT_SIZE_TITLE, titleColor);

  int subtitleWidth = MeasureText(subtitleText, FONT_SIZE_LARGE);
  DrawText(subtitleText, panelX + (panelWidth - subtitleWidth) / 2, panelY + 90,
           FONT_SIZE_LARGE, WHITE);

  const char *restartText = "Press R to Play Again";
  int restartWidth = MeasureText(restartText, FONT_SIZE_MEDIUM);
  DrawText(restartText, panelX + (panelWidth - restartWidth) / 2, panelY + 150,
           FONT_SIZE_MEDIUM, LIGHTGRAY);
}

//==============================================================================
// INPUT HANDLING
//==============================================================================

void HandleInput(void) {
  if (IsKeyPressed(KEY_R)) {
    InitBoard();
    isDragging = false;
    return;
  }

  if (gameState == GAME_CHECKMATE || gameState == GAME_STALEMATE) {
    return;
  }

  Vector2 mouse = GetMousePosition();
  int col = (mouse.x - BOARD_OFFSET_X) / TILE_SIZE;
  int row = (mouse.y - BOARD_OFFSET_Y) / TILE_SIZE;

  // Start dragging or click to select
  if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    if (IsValidPosition(row, col)) {
      if (board[row][col].type != PIECE_NONE &&
          board[row][col].color == currentTurn) {
        // Start dragging this piece
        isDragging = true;
        dragStartPos = (Position){row, col};
        selectedPos = (Position){row, col};
        ClearValidMoves();
        CalculateValidMoves(row, col);

        // Calculate offset from piece center for smooth dragging
        int pieceX =
            BOARD_OFFSET_X + col * TILE_SIZE + (TILE_SIZE - SPRITE_SIZE) / 2;
        int pieceY =
            BOARD_OFFSET_Y + row * TILE_SIZE + (TILE_SIZE - SPRITE_SIZE) / 2;
        dragOffset = (Vector2){mouse.x - pieceX - SPRITE_SIZE / 2,
                               mouse.y - pieceY - SPRITE_SIZE / 2};
      } else if (selectedPos.row != -1 && IsValidMove(row, col)) {
        // Click on valid move square
        MovePiece(row, col);
      } else {
        // Deselect
        selectedPos = (Position){-1, -1};
        ClearValidMoves();
      }
    }
  }

  // Release to drop piece
  if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && isDragging) {
    isDragging = false;

    if (IsValidPosition(row, col)) {
      if (IsValidMove(row, col)) {
        MovePiece(row, col);
      } else if (row != dragStartPos.row || col != dragStartPos.col) {
        // Invalid drop - deselect
        selectedPos = (Position){-1, -1};
        ClearValidMoves();
      }
      // If dropped on same square, keep selected
    } else {
      // Dropped outside board - deselect
      selectedPos = (Position){-1, -1};
      ClearValidMoves();
    }
  }
}

void HandlePromotion(void) {
  if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    return;

  Vector2 mouse = GetMousePosition();
  PanelLayout panel = GetPromotionPanelLayout();
  PieceType options[] = {PIECE_QUEEN, PIECE_ROOK, PIECE_BISHOP, PIECE_KNIGHT};

  for (int i = 0; i < 4; i++) {
    int x = panel.x + PANEL_PADDING + i * (TILE_SIZE + BUTTON_SPACING);
    int y = panel.y + 45;

    if (mouse.x >= x && mouse.x < x + TILE_SIZE && mouse.y >= y &&
        mouse.y < y + TILE_SIZE) {
      // Promote the piece
      board[promotionPos.row][promotionPos.col].type = options[i];

      // Record the promotion move
      RecordMove(promotionFromPos.row, promotionFromPos.col, promotionPos.row,
                 promotionPos.col, promotionWasCapture, false, false, false,
                 true, options[i]);

      // Switch turns and update game state
      currentTurn = OPPONENT_COLOR(currentTurn);
      gameState = GAME_PLAYING;
      promotionPos = (Position){-1, -1};
      promotionFromPos = (Position){-1, -1};
      UpdateGameState();

      // Update move history with check/checkmate status
      UpdateLastMoveStatus(gameState == GAME_CHECK ||
                               gameState == GAME_CHECKMATE,
                           gameState == GAME_CHECKMATE);
      break;
    }
  }
}
