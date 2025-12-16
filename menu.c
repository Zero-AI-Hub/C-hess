/**
 * Chess Game - Menu Screens
 * Title screen and options menu.
 */

#include "menu.h"
#include "board.h"
#include "ui.h"

//==============================================================================
// FLOATING PIECE ANIMATION
//==============================================================================

typedef struct {
  float x, y;
  float speedX, speedY;
  float rotation;
  float rotSpeed;
  PieceType type;
  PieceColor color;
  float alpha;
} FloatingPiece;

static FloatingPiece floatingPieces[MAX_FLOATING_PIECES];

void InitFloatingPieces(void) {
  PieceType types[] = {PIECE_KING,   PIECE_QUEEN,  PIECE_ROOK,
                       PIECE_BISHOP, PIECE_KNIGHT, PIECE_PAWN};

  for (int i = 0; i < MAX_FLOATING_PIECES; i++) {
    floatingPieces[i].x = GetRandomValue(0, WINDOW_WIDTH);
    floatingPieces[i].y = GetRandomValue(0, WINDOW_HEIGHT);
    floatingPieces[i].speedX = GetRandomValue(-20, 20) / 10.0f;
    floatingPieces[i].speedY = GetRandomValue(-20, 20) / 10.0f;
    floatingPieces[i].rotation = GetRandomValue(0, 360);
    floatingPieces[i].rotSpeed = GetRandomValue(-30, 30) / 10.0f;
    floatingPieces[i].type = types[GetRandomValue(0, 5)];
    floatingPieces[i].color = (i % 2 == 0) ? COLOR_WHITE : COLOR_BLACK;
    floatingPieces[i].alpha = 0.15f + GetRandomValue(0, 15) / 100.0f;
  }
}

void UpdateFloatingPieces(void) {
  for (int i = 0; i < MAX_FLOATING_PIECES; i++) {
    floatingPieces[i].x += floatingPieces[i].speedX;
    floatingPieces[i].y += floatingPieces[i].speedY;
    floatingPieces[i].rotation += floatingPieces[i].rotSpeed;

    // Wrap around screen edges
    if (floatingPieces[i].x < -SPRITE_SIZE)
      floatingPieces[i].x = WINDOW_WIDTH + SPRITE_SIZE;
    if (floatingPieces[i].x > WINDOW_WIDTH + SPRITE_SIZE)
      floatingPieces[i].x = -SPRITE_SIZE;
    if (floatingPieces[i].y < -SPRITE_SIZE)
      floatingPieces[i].y = WINDOW_HEIGHT + SPRITE_SIZE;
    if (floatingPieces[i].y > WINDOW_HEIGHT + SPRITE_SIZE)
      floatingPieces[i].y = -SPRITE_SIZE;
  }
}

static void DrawFloatingPieces(void) {
  for (int i = 0; i < MAX_FLOATING_PIECES; i++) {
    Rectangle src =
        GetSpriteRect(floatingPieces[i].type, floatingPieces[i].color);
    Rectangle dest = {floatingPieces[i].x, floatingPieces[i].y,
                      SPRITE_SIZE * 1.5f, SPRITE_SIZE * 1.5f};
    Vector2 origin = {SPRITE_SIZE * 0.75f, SPRITE_SIZE * 0.75f};

    unsigned char alpha = (unsigned char)(floatingPieces[i].alpha * 255);
    Color tint = {255, 255, 255, alpha};

    DrawTexturePro(piecesTexture, src, dest, origin, floatingPieces[i].rotation,
                   tint);
  }
}

//==============================================================================
// MENU BUTTON
//==============================================================================

bool DrawMenuButton(int x, int y, int width, int height, const char *text) {
  Vector2 mouse = GetMousePosition();
  Rectangle rect = {x, y, width, height};
  bool hover = CheckCollisionPointRec(mouse, rect);
  bool clicked = hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

  // Button background with hover effect
  Color bgColor = hover ? COLOR_BUTTON_HOVER : COLOR_BUTTON_BG;
  DrawRectangle(x, y, width, height, bgColor);
  DrawRectangleLinesEx(rect, 2, WHITE);

  // Centered text
  int textWidth = MeasureText(text, FONT_SIZE_LARGE);
  int textX = x + (width - textWidth) / 2;
  int textY = y + (height - FONT_SIZE_LARGE) / 2;
  DrawText(text, textX, textY, FONT_SIZE_LARGE, WHITE);

  return clicked;
}

//==============================================================================
// TITLE SCREEN
//==============================================================================

void DrawTitleScreen(void) {
  // Draw decorative checkerboard pattern (faded) covering full window
  int cols = (WINDOW_WIDTH + TILE_SIZE - 1) / TILE_SIZE;
  int rows = (WINDOW_HEIGHT + TILE_SIZE - 1) / TILE_SIZE;
  for (int row = 0; row < rows; row++) {
    for (int col = 0; col < cols; col++) {
      int x = col * TILE_SIZE;
      int y = row * TILE_SIZE;
      unsigned char alpha = ((row + col) % 2 == 0) ? 15 : 25;
      Color squareColor = {100, 80, 60, alpha};
      DrawRectangle(x, y, TILE_SIZE, TILE_SIZE, squareColor);
    }
  }

  // Draw floating pieces
  DrawFloatingPieces();

  // Title shadow
  const char *title = "CHESS";
  int titleWidth = MeasureText(title, 80);
  int titleX = (WINDOW_WIDTH - titleWidth) / 2;
  int titleY = 80;

  DrawText(title, titleX + 4, titleY + 4, 80, COLOR_TITLE_SHADOW);
  DrawText(title, titleX, titleY, 80, COLOR_TITLE_GOLD);

  // Decorative line under title
  DrawRectangle(titleX - 20, titleY + 90, titleWidth + 40, 4, COLOR_TITLE_GOLD);

  // Menu buttons
  int buttonWidth = 200;
  int buttonHeight = 50;
  int buttonX = (WINDOW_WIDTH - buttonWidth) / 2;
  int buttonY = 380;

  if (DrawMenuButton(buttonX, buttonY, buttonWidth, buttonHeight, "PLAY")) {
    InitBoard();
    currentScreen = SCREEN_GAME;
  }

  if (DrawMenuButton(buttonX, buttonY + 70, buttonWidth, buttonHeight,
                     "OPTIONS")) {
    currentScreen = SCREEN_OPTIONS;
  }

  // Footer text
  const char *footer = "Press ESC during game to return";
  int footerWidth = MeasureText(footer, FONT_SIZE_SMALL);
  DrawText(footer, (WINDOW_WIDTH - footerWidth) / 2, WINDOW_HEIGHT - 40,
           FONT_SIZE_SMALL, GRAY);
}

void HandleTitleInput(void) {
  // Button clicks are handled in DrawMenuButton via IsMouseButtonPressed
}

//==============================================================================
// OPTIONS SCREEN
//==============================================================================

void DrawOptionsScreen(void) {
  // Darken background
  DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, COLOR_OVERLAY_DARK);

  // Options panel
  int panelWidth = 350;
  int panelHeight = 300;
  int panelX = (WINDOW_WIDTH - panelWidth) / 2;
  int panelY = (WINDOW_HEIGHT - panelHeight) / 2;

  DrawRectangle(panelX, panelY, panelWidth, panelHeight, COLOR_PANEL_BG);
  DrawRectangleLinesEx((Rectangle){panelX, panelY, panelWidth, panelHeight}, 3,
                       WHITE);

  // Title
  const char *optionsTitle = "OPTIONS";
  int titleWidth = MeasureText(optionsTitle, FONT_SIZE_TITLE);
  DrawText(optionsTitle, panelX + (panelWidth - titleWidth) / 2, panelY + 20,
           FONT_SIZE_TITLE, WHITE);

  // Placeholder content
  const char *placeholder = "Settings coming soon...";
  int placeholderWidth = MeasureText(placeholder, FONT_SIZE_MEDIUM);
  DrawText(placeholder, panelX + (panelWidth - placeholderWidth) / 2,
           panelY + 100, FONT_SIZE_MEDIUM, GRAY);

  // Controls info
  DrawText("Controls:", panelX + 25, panelY + 150, FONT_SIZE_SMALL, WHITE);
  DrawText("- Click/drag to move pieces", panelX + 25, panelY + 175,
           FONT_SIZE_SMALL, LIGHTGRAY);
  DrawText("- Press R to restart", panelX + 25, panelY + 195, FONT_SIZE_SMALL,
           LIGHTGRAY);
  DrawText("- Press ESC for menu", panelX + 25, panelY + 215, FONT_SIZE_SMALL,
           LIGHTGRAY);

  // Back button
  int buttonWidth = 150;
  int buttonHeight = 40;
  int buttonX = panelX + (panelWidth - buttonWidth) / 2;
  int buttonY = panelY + panelHeight - 60;

  if (DrawMenuButton(buttonX, buttonY, buttonWidth, buttonHeight, "BACK")) {
    currentScreen = SCREEN_TITLE;
  }
}

void HandleOptionsInput(void) {
  // ESC is handled globally in main loop
}
