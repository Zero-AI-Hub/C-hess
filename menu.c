/**
 * Chess Game - Menu Screens
 * Title screen and options menu.
 */

#include "menu.h"
#include "board.h"
#include "clock.h"
#include "ui.h"
#include <stdio.h>

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
  int titleWidth = MeasureText(title, FONT_SIZE_TITLE_LARGE);
  int titleX = (WINDOW_WIDTH - titleWidth) / 2;
  int titleY = FONT_SIZE_TITLE_LARGE;

  DrawText(title, titleX + 4, titleY + 4, FONT_SIZE_TITLE_LARGE,
           COLOR_TITLE_SHADOW);
  DrawText(title, titleX, titleY, FONT_SIZE_TITLE_LARGE, COLOR_TITLE_GOLD);

  // Decorative line under title
  DrawRectangle(titleX - 20, titleY + 90, titleWidth + 40, 4, COLOR_TITLE_GOLD);

  // Menu buttons
  int buttonWidth = MENU_BUTTON_WIDTH;
  int buttonHeight = MENU_BUTTON_HEIGHT;
  int buttonX = (WINDOW_WIDTH - buttonWidth) / 2;
  int buttonY = MENU_BUTTON_Y_START;

  if (DrawMenuButton(buttonX, buttonY, buttonWidth, buttonHeight, "PLAY")) {
    InitClockConfig();
    currentScreen = SCREEN_CLOCK_SETUP;
  }

  if (DrawMenuButton(buttonX, buttonY + MENU_BUTTON_Y_SPACING, buttonWidth,
                     buttonHeight, "OPTIONS")) {
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

//==============================================================================
// CLOCK SETUP SCREEN
//==============================================================================

// Time control presets: {minutes, seconds, increment, type, label}
typedef struct {
  int minutes;
  int seconds;
  int increment;
  ClockType type;
  const char *label;
} TimePreset;

static const TimePreset presets[] = {
    {1, 0, 0, CLOCK_SUDDEN_DEATH, "1+0"},
    {1, 0, 1, CLOCK_FISCHER, "1+1"},
    {2, 0, 1, CLOCK_FISCHER, "2+1"},
    {3, 0, 0, CLOCK_SUDDEN_DEATH, "3+0"},
    {3, 0, 2, CLOCK_FISCHER, "3+2"},
    {5, 0, 0, CLOCK_SUDDEN_DEATH, "5+0"},
    {5, 0, 3, CLOCK_FISCHER, "5+3"},
    {10, 0, 0, CLOCK_SUDDEN_DEATH, "10+0"},
    {10, 0, 5, CLOCK_FISCHER, "10+5"},
    {15, 0, 10, CLOCK_FISCHER, "15+10"},
};
#define NUM_PRESETS (sizeof(presets) / sizeof(presets[0]))

static const char *clockTypeNames[] = {"None", "Sudden Death", "Fischer",
                                       "Simple Delay", "Bronstein"};
#define NUM_CLOCK_TYPES 5

static int selectedPreset = 5; // Default: 5+0
static bool customMode = false;

// Helper to draw a small button
static bool DrawSmallButton(int x, int y, int width, int height,
                            const char *text, bool selected) {
  Vector2 mouse = GetMousePosition();
  Rectangle rect = {x, y, width, height};
  bool hover = CheckCollisionPointRec(mouse, rect);
  bool clicked = hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

  Color bgColor = selected ? COLOR_TITLE_GOLD
                           : (hover ? COLOR_BUTTON_HOVER : COLOR_BUTTON_BG);
  DrawRectangle(x, y, width, height, bgColor);
  DrawRectangleLinesEx(rect, 1, WHITE);

  int fontSize = FONT_SIZE_SMALL;
  int textWidth = MeasureText(text, fontSize);
  DrawText(text, x + (width - textWidth) / 2, y + (height - fontSize) / 2,
           fontSize, selected ? BLACK : WHITE);

  return clicked;
}

// Helper to draw a slider and return new value
static int DrawSlider(int x, int y, int width, const char *label, int value,
                      int minVal, int maxVal) {
  // Draw label
  DrawText(label, x, y, FONT_SIZE_SMALL, WHITE);

  // Draw slider track
  int sliderY = y + 25;
  int sliderHeight = 8;
  DrawRectangle(x, sliderY, width, sliderHeight, DARKGRAY);

  // Calculate and draw slider handle
  float ratio = (float)(value - minVal) / (float)(maxVal - minVal);
  int handleX = x + (int)(ratio * (width - 10));
  int handleWidth = 10;
  int handleHeight = 20;
  DrawRectangle(handleX, sliderY - 6, handleWidth, handleHeight,
                COLOR_TITLE_GOLD);

  // Draw value
  char valueStr[16];
  sprintf(valueStr, "%d", value);
  DrawText(valueStr, x + width + 10, y, FONT_SIZE_SMALL, WHITE);

  // Handle input
  Vector2 mouse = GetMousePosition();
  Rectangle sliderRect = {x - 5, sliderY - 10, width + 10, handleHeight + 10};
  if (CheckCollisionPointRec(mouse, sliderRect) &&
      IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
    float newRatio = (mouse.x - x) / (float)width;
    if (newRatio < 0)
      newRatio = 0;
    if (newRatio > 1)
      newRatio = 1;
    value = minVal + (int)(newRatio * (maxVal - minVal));
  }

  return value;
}

void DrawClockSetupScreen(void) {
  // Darken background
  DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, COLOR_OVERLAY_DARK);

  // Panel
  int panelWidth = 450;
  int panelHeight = 480;
  int panelX = (WINDOW_WIDTH - panelWidth) / 2;
  int panelY = (WINDOW_HEIGHT - panelHeight) / 2;

  DrawRectangle(panelX, panelY, panelWidth, panelHeight, COLOR_PANEL_BG);
  DrawRectangleLinesEx((Rectangle){panelX, panelY, panelWidth, panelHeight}, 3,
                       WHITE);

  // Title
  const char *title = "TIME CONTROL";
  int titleWidth = MeasureText(title, FONT_SIZE_TITLE);
  DrawText(title, panelX + (panelWidth - titleWidth) / 2, panelY + 15,
           FONT_SIZE_TITLE, WHITE);

  // Presets section
  int contentX = panelX + 25;
  int contentY = panelY + 70;

  DrawText("Quick Select:", contentX, contentY, FONT_SIZE_SMALL, LIGHTGRAY);
  contentY += 25;

  // Draw preset buttons in a grid (5 per row)
  int btnWidth = 70;
  int btnHeight = 35;
  int btnSpacing = 8;
  int buttonsPerRow = 5;

  for (int i = 0; i < (int)NUM_PRESETS; i++) {
    int row = i / buttonsPerRow;
    int col = i % buttonsPerRow;
    int btnX = contentX + col * (btnWidth + btnSpacing);
    int btnY = contentY + row * (btnHeight + btnSpacing);

    bool isSelected = !customMode && selectedPreset == i;
    if (DrawSmallButton(btnX, btnY, btnWidth, btnHeight, presets[i].label,
                        isSelected)) {
      selectedPreset = i;
      customMode = false;
      clockConfig.baseMinutes = presets[i].minutes;
      clockConfig.baseSeconds = presets[i].seconds;
      clockConfig.incrementSeconds = presets[i].increment;
      clockConfig.selectedType = presets[i].type;
    }
  }

  contentY += 2 * (btnHeight + btnSpacing) + 20;

  // Custom button
  if (DrawSmallButton(contentX, contentY, 100, 35, "Custom", customMode)) {
    customMode = true;
  }

  contentY += 50;

  // Clock type selector
  DrawText("Clock Type:", contentX, contentY, FONT_SIZE_SMALL, LIGHTGRAY);
  contentY += 25;

  int typeBtnWidth = 80;
  for (int i = 0; i < NUM_CLOCK_TYPES; i++) {
    int btnX = contentX + i * (typeBtnWidth + 4);
    bool isSelected = (int)clockConfig.selectedType == i;
    if (DrawSmallButton(btnX, contentY, typeBtnWidth, 30, clockTypeNames[i],
                        isSelected)) {
      clockConfig.selectedType = (ClockType)i;
      customMode = true; // Selecting type switches to custom mode
    }
  }

  contentY += 45;

  // Custom time sliders (only shown in custom mode or always visible for
  // clarity)
  int sliderWidth = 300;

  clockConfig.baseMinutes =
      DrawSlider(contentX, contentY, sliderWidth,
                 "Minutes:", clockConfig.baseMinutes, 0, 60);
  contentY += 55;

  clockConfig.baseSeconds =
      DrawSlider(contentX, contentY, sliderWidth,
                 "Seconds:", clockConfig.baseSeconds, 0, 59);
  contentY += 55;

  const char *incLabel = (clockConfig.selectedType == CLOCK_SIMPLE_DELAY ||
                          clockConfig.selectedType == CLOCK_BRONSTEIN)
                             ? "Delay (sec):"
                             : "Increment (sec):";
  clockConfig.incrementSeconds =
      DrawSlider(contentX, contentY, sliderWidth, incLabel,
                 clockConfig.incrementSeconds, 0, 30);

  contentY += 60;

  // Current selection display
  char timeStr[64];
  if (clockConfig.selectedType == CLOCK_NONE) {
    sprintf(timeStr, "No time limit");
  } else {
    int totalSec = clockConfig.baseMinutes * 60 + clockConfig.baseSeconds;
    int mins = totalSec / 60;
    int secs = totalSec % 60;
    if (secs > 0) {
      sprintf(timeStr, "%d:%02d + %ds (%s)", mins, secs,
              clockConfig.incrementSeconds,
              clockTypeNames[clockConfig.selectedType]);
    } else {
      sprintf(timeStr, "%d + %ds (%s)", mins, clockConfig.incrementSeconds,
              clockTypeNames[clockConfig.selectedType]);
    }
  }
  int timeStrWidth = MeasureText(timeStr, FONT_SIZE_MEDIUM);
  DrawText(timeStr, panelX + (panelWidth - timeStrWidth) / 2,
           panelY + panelHeight - 90, FONT_SIZE_MEDIUM, COLOR_TITLE_GOLD);

  // Start Game button
  int startBtnWidth = 180;
  int startBtnHeight = 45;
  int startBtnX = panelX + (panelWidth - startBtnWidth) / 2;
  int startBtnY = panelY + panelHeight - 55;

  if (DrawMenuButton(startBtnX, startBtnY, startBtnWidth, startBtnHeight,
                     "START GAME")) {
    InitBoard();
    InitClock();
    StartClock();
    currentScreen = SCREEN_GAME;
  }
}

void HandleClockSetupInput(void) {
  // ESC returns to title
  if (IsKeyPressed(KEY_ESCAPE)) {
    currentScreen = SCREEN_TITLE;
  }
}
