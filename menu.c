/**
 * Chess Game - Menu Screens
 * Title screen and options menu.
 */

#include "menu.h"
#include "board.h"
#include "clock.h"
#include "multiplayer.h"
#include "network.h"
#include "ui.h"
#include <stdio.h>
#include <string.h>

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

  // Centered text with auto-sizing font
  int fontSize = FONT_SIZE_LARGE;
  int textWidth = MeasureText(text, fontSize);
  while (textWidth > width - 16 && fontSize > 14) {
    fontSize -= 2;
    textWidth = MeasureText(text, fontSize);
  }
  int textX = x + (width - textWidth) / 2;
  int textY = y + (height - fontSize) / 2;
  DrawText(text, textX, textY, fontSize, WHITE);

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

  // Only show multiplayer and options buttons if we're on title screen (not
  // popup)
  if (currentScreen == SCREEN_TITLE) {
    if (DrawMenuButton(buttonX, buttonY + MENU_BUTTON_Y_SPACING, buttonWidth,
                       buttonHeight, "MULTIPLAYER")) {
      currentScreen = SCREEN_MULTIPLAYER;
    }

    if (DrawMenuButton(buttonX, buttonY + MENU_BUTTON_Y_SPACING * 2,
                       buttonWidth, buttonHeight, "OPTIONS")) {
      currentScreen = SCREEN_OPTIONS;
    }
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

// Helper to draw a small button with auto-sizing text
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

  // Auto-size font to fit button width
  int fontSize = FONT_SIZE_SMALL;
  int textWidth = MeasureText(text, fontSize);
  while (textWidth > width - 8 && fontSize > 10) {
    fontSize -= 2;
    textWidth = MeasureText(text, fontSize);
  }

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

  // Panel - tall enough to fit all elements with comfortable spacing
  int panelWidth = 450;
  int panelHeight = 620;
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

  // Current selection display - positioned with space above the button
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
           panelY + panelHeight - 110, FONT_SIZE_MEDIUM, COLOR_TITLE_GOLD);

  // Start Game button - at the bottom with clear spacing from hint
  int startBtnWidth = 180;
  int startBtnHeight = 45;
  int startBtnX = panelX + (panelWidth - startBtnWidth) / 2;
  int startBtnY = panelY + panelHeight - 60;

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

//==============================================================================
// MULTIPLAYER SCREENS
//==============================================================================

// Text input buffer for multiplayer codes
static char inputBuffer[NET_CODE_MAX_LEN] = {0};
static int inputCursor = 0;

// Helper to truncate text to fit within a given pixel width
static void TruncateTextToWidth(const char *text, int maxWidth, int fontSize,
                                char *output, int outputSize) {
  int textWidth = MeasureText(text, fontSize);
  if (textWidth <= maxWidth) {
    strncpy(output, text, outputSize - 1);
    output[outputSize - 1] = '\0';
    return;
  }

  // Need to truncate - show beginning...end
  const char *ellipsis = "...";
  int ellipsisWidth = MeasureText(ellipsis, fontSize);
  int availableWidth = maxWidth - ellipsisWidth;

  // Find how many chars from start fit in half the space
  int halfWidth = availableWidth / 2;
  int startChars = 0;
  char temp[256];
  for (int i = 1; i <= (int)strlen(text) && i < 100; i++) {
    strncpy(temp, text, i);
    temp[i] = '\0';
    if (MeasureText(temp, fontSize) > halfWidth)
      break;
    startChars = i;
  }

  // Find how many chars from end fit in the other half
  int textLen = strlen(text);
  int endChars = 0;
  for (int i = 1; i <= textLen && i < 100; i++) {
    strncpy(temp, text + textLen - i, i);
    temp[i] = '\0';
    if (MeasureText(temp, fontSize) > halfWidth)
      break;
    endChars = i;
  }

  // Build the truncated string
  if (startChars > 0 && endChars > 0) {
    snprintf(output, outputSize, "%.*s...%s", startChars, text,
             text + textLen - endChars);
  } else {
    // Fallback: just truncate start
    strncpy(output, text, outputSize - 4);
    output[outputSize - 4] = '\0';
    strcat(output, "...");
  }
}

// Helper to draw a text input field
static void DrawTextInput(int x, int y, int width, int height,
                          const char *buffer, int cursor, const char *hint) {
  (void)cursor; // Cursor position not used for truncated display
  // Draw input box
  DrawRectangle(x, y, width, height, DARKGRAY);
  DrawRectangleLinesEx((Rectangle){x, y, width, height}, 2, WHITE);

  // Draw text or hint
  int fontSize = FONT_SIZE_SMALL;
  int padding = 10;
  int maxTextWidth = width - padding * 2;

  if (strlen(buffer) > 0) {
    // Truncate display to fit within box
    char display[128];
    TruncateTextToWidth(buffer, maxTextWidth, fontSize, display,
                        sizeof(display));
    DrawText(display, x + padding, y + (height - fontSize) / 2, fontSize,
             WHITE);
  } else {
    DrawText(hint, x + padding, y + (height - fontSize) / 2, fontSize, GRAY);
  }

  // Draw cursor at end of visible area
  if ((int)(GetTime() * 2) % 2 == 0 && strlen(buffer) > 0) {
    char display[128];
    TruncateTextToWidth(buffer, maxTextWidth, fontSize, display,
                        sizeof(display));
    int displayWidth = MeasureText(display, fontSize);
    DrawRectangle(x + padding + displayWidth + 2, y + 5, 2, height - 10, WHITE);
  }
}

// Handle text input
static void HandleTextInput(char *buffer, int *cursor, int maxLen) {
  // Handle character input
  int key = GetCharPressed();
  while (key > 0) {
    if ((key >= 32) && (key <= 125) && (*cursor < maxLen - 1)) {
      buffer[*cursor] = (char)key;
      buffer[*cursor + 1] = '\0';
      (*cursor)++;
    }
    key = GetCharPressed();
  }

  // Handle backspace
  if (IsKeyPressed(KEY_BACKSPACE) && *cursor > 0) {
    (*cursor)--;
    buffer[*cursor] = '\0';
  }

  // Handle paste (Ctrl+V)
  if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) &&
      IsKeyPressed(KEY_V)) {
    const char *clipboard = GetClipboardText();
    if (clipboard) {
      int clipLen = strlen(clipboard);
      int copyLen =
          (clipLen < maxLen - *cursor - 1) ? clipLen : maxLen - *cursor - 1;
      memcpy(buffer + *cursor, clipboard, copyLen);
      *cursor += copyLen;
      buffer[*cursor] = '\0';
    }
  }
}

void DrawMultiplayerScreen(void) {
  // Darken background
  DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, COLOR_OVERLAY_DARK);

  // Panel
  int panelWidth = 400;
  int panelHeight = 350;
  int panelX = (WINDOW_WIDTH - panelWidth) / 2;
  int panelY = (WINDOW_HEIGHT - panelHeight) / 2;

  DrawRectangle(panelX, panelY, panelWidth, panelHeight, COLOR_PANEL_BG);
  DrawRectangleLinesEx((Rectangle){panelX, panelY, panelWidth, panelHeight}, 3,
                       WHITE);

  // Title
  const char *title = "MULTIPLAYER";
  int titleWidth = MeasureText(title, FONT_SIZE_TITLE);
  DrawText(title, panelX + (panelWidth - titleWidth) / 2, panelY + 20,
           FONT_SIZE_TITLE, WHITE);

  // Description
  const char *desc = "Play chess with a friend online!";
  int descWidth = MeasureText(desc, FONT_SIZE_SMALL);
  DrawText(desc, panelX + (panelWidth - descWidth) / 2, panelY + 75,
           FONT_SIZE_SMALL, LIGHTGRAY);

  // Buttons
  int buttonWidth = 200;
  int buttonHeight = 50;
  int buttonX = panelX + (panelWidth - buttonWidth) / 2;

  if (DrawMenuButton(buttonX, panelY + 120, buttonWidth, buttonHeight,
                     "CREATE GAME")) {
    memset(inputBuffer, 0, sizeof(inputBuffer));
    inputCursor = 0;
    multiplayerRole = MP_ROLE_HOST;
    CreateHostSession();
    currentScreen = SCREEN_MP_HOST;
  }

  if (DrawMenuButton(buttonX, panelY + 190, buttonWidth, buttonHeight,
                     "JOIN GAME")) {
    memset(inputBuffer, 0, sizeof(inputBuffer));
    inputCursor = 0;
    multiplayerRole = MP_ROLE_GUEST;
    currentScreen = SCREEN_MP_JOIN;
  }

  // Back button
  if (DrawMenuButton(buttonX, panelY + panelHeight - 70, buttonWidth, 40,
                     "BACK")) {
    currentScreen = SCREEN_TITLE;
  }
}

void HandleMultiplayerInput(void) {
  if (IsKeyPressed(KEY_ESCAPE)) {
    currentScreen = SCREEN_TITLE;
  }
}

void DrawHostScreen(void) {
  // Darken background
  DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, COLOR_OVERLAY_DARK);

  // Panel
  int panelWidth = 500;
  int panelHeight = 480;
  int panelX = (WINDOW_WIDTH - panelWidth) / 2;
  int panelY = (WINDOW_HEIGHT - panelHeight) / 2;

  DrawRectangle(panelX, panelY, panelWidth, panelHeight, COLOR_PANEL_BG);
  DrawRectangleLinesEx((Rectangle){panelX, panelY, panelWidth, panelHeight}, 3,
                       WHITE);

  // Title
  const char *title = "CREATE GAME";
  int titleWidth = MeasureText(title, FONT_SIZE_TITLE);
  DrawText(title, panelX + (panelWidth - titleWidth) / 2, panelY + 15,
           FONT_SIZE_TITLE, WHITE);

  int contentX = panelX + 25;
  int contentY = panelY + 70;

  // Status
  const char *status = GetNetworkStatusString();
  DrawText("Status:", contentX, contentY, FONT_SIZE_SMALL, LIGHTGRAY);
  DrawText(status, contentX + 80, contentY, FONT_SIZE_SMALL, COLOR_TITLE_GOLD);
  contentY += 30;

  NetworkState state = GetNetworkState();

  if (state == NET_WAITING_ANSWER || state == NET_CONNECTING ||
      state == NET_CONNECTED) {
    // Show offer code
    DrawText("Your Offer Code:", contentX, contentY, FONT_SIZE_SMALL, WHITE);
    contentY += 25;

    // Display truncated code to fit in box
    int codeBoxWidth = panelWidth - 50;
    char displayCode[128];
    TruncateTextToWidth(localOfferCode, codeBoxWidth - 10, FONT_SIZE_SMALL,
                        displayCode, sizeof(displayCode));
    DrawRectangle(contentX, contentY, codeBoxWidth, 30, DARKGRAY);
    DrawRectangleLinesEx((Rectangle){contentX, contentY, codeBoxWidth, 30}, 1,
                         GRAY);
    DrawText(displayCode, contentX + 5, contentY + 5, FONT_SIZE_SMALL, WHITE);
    contentY += 35;

    // Copy button
    if (DrawMenuButton(contentX, contentY, 150, 35, "COPY CODE")) {
      SetClipboardText(localOfferCode);
    }
    contentY += 50;

    // Instructions
    DrawText("1. Copy the code above", contentX, contentY, FONT_SIZE_SMALL,
             LIGHTGRAY);
    contentY += 20;
    DrawText("2. Send it to your friend", contentX, contentY, FONT_SIZE_SMALL,
             LIGHTGRAY);
    contentY += 20;
    DrawText("3. Paste their answer code below", contentX, contentY,
             FONT_SIZE_SMALL, LIGHTGRAY);
    contentY += 30;

    // Answer code input
    DrawText("Friend's Answer Code:", contentX, contentY, FONT_SIZE_SMALL,
             WHITE);
    contentY += 25;

    DrawTextInput(contentX, contentY, panelWidth - 50, 35, inputBuffer,
                  inputCursor, "Paste answer code here...");
    contentY += 45;

    // Connect button
    if (strlen(inputBuffer) > 10) {
      if (DrawMenuButton(contentX, contentY, 150, 35, "CONNECT")) {
        SetAnswerCode(inputBuffer);
      }
    }
  } else if (state == NET_GATHERING) {
    DrawText("Gathering network information...", contentX, contentY,
             FONT_SIZE_SMALL, LIGHTGRAY);
    DrawText("Please wait...", contentX, contentY + 25, FONT_SIZE_SMALL, GRAY);
  } else if (state == NET_FAILED) {
    DrawText("Connection failed!", contentX, contentY, FONT_SIZE_SMALL, RED);
    DrawText("Please try again.", contentX, contentY + 25, FONT_SIZE_SMALL,
             GRAY);
  }

  // Check if connected - start the game
  if (state == NET_CONNECTED) {
    StartMultiplayerGame();
    currentScreen = SCREEN_GAME;
  }

  // Back button
  if (DrawMenuButton(panelX + (panelWidth - 150) / 2, panelY + panelHeight - 50,
                     150, 35, "CANCEL")) {
    DisconnectNetwork();
    ResetMultiplayer();
    currentScreen = SCREEN_MULTIPLAYER;
  }
}

void HandleHostInput(void) {
  if (IsKeyPressed(KEY_ESCAPE)) {
    DisconnectNetwork();
    ResetMultiplayer();
    currentScreen = SCREEN_MULTIPLAYER;
  }

  // Handle text input for answer code
  HandleTextInput(inputBuffer, &inputCursor, NET_CODE_MAX_LEN);
}

void DrawJoinScreen(void) {
  // Darken background
  DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, COLOR_OVERLAY_DARK);

  // Panel
  int panelWidth = 500;
  int panelHeight = 450;
  int panelX = (WINDOW_WIDTH - panelWidth) / 2;
  int panelY = (WINDOW_HEIGHT - panelHeight) / 2;

  DrawRectangle(panelX, panelY, panelWidth, panelHeight, COLOR_PANEL_BG);
  DrawRectangleLinesEx((Rectangle){panelX, panelY, panelWidth, panelHeight}, 3,
                       WHITE);

  // Title
  const char *title = "JOIN GAME";
  int titleWidth = MeasureText(title, FONT_SIZE_TITLE);
  DrawText(title, panelX + (panelWidth - titleWidth) / 2, panelY + 15,
           FONT_SIZE_TITLE, WHITE);

  int contentX = panelX + 25;
  int contentY = panelY + 70;

  // Status
  const char *status = GetNetworkStatusString();
  DrawText("Status:", contentX, contentY, FONT_SIZE_SMALL, LIGHTGRAY);
  DrawText(status, contentX + 80, contentY, FONT_SIZE_SMALL, COLOR_TITLE_GOLD);
  contentY += 30;

  NetworkState state = GetNetworkState();

  if (state == NET_DISCONNECTED) {
    // Input for host's offer code
    DrawText("Host's Offer Code:", contentX, contentY, FONT_SIZE_SMALL, WHITE);
    contentY += 25;

    DrawTextInput(contentX, contentY, panelWidth - 50, 35, inputBuffer,
                  inputCursor, "Paste offer code here...");
    contentY += 50;

    // Connect button
    if (strlen(inputBuffer) > 10) {
      if (DrawMenuButton(contentX, contentY, 150, 35, "CONNECT")) {
        JoinSession(inputBuffer);
      }
    }
  } else if (state == NET_GATHERING) {
    DrawText("Connecting to host...", contentX, contentY, FONT_SIZE_SMALL,
             LIGHTGRAY);
    DrawText("Generating answer code...", contentX, contentY + 25,
             FONT_SIZE_SMALL, GRAY);
  } else if (state == NET_WAITING_CONNECTION || state == NET_CONNECTING) {
    // Show answer code
    DrawText("Your Answer Code:", contentX, contentY, FONT_SIZE_SMALL, WHITE);
    contentY += 25;

    // Display truncated code to fit in box
    int codeBoxWidth = panelWidth - 50;
    char displayCode[128];
    TruncateTextToWidth(localAnswerCode, codeBoxWidth - 10, FONT_SIZE_SMALL,
                        displayCode, sizeof(displayCode));
    DrawRectangle(contentX, contentY, codeBoxWidth, 30, DARKGRAY);
    DrawRectangleLinesEx((Rectangle){contentX, contentY, codeBoxWidth, 30}, 1,
                         GRAY);
    DrawText(displayCode, contentX + 5, contentY + 5, FONT_SIZE_SMALL, WHITE);
    contentY += 35;

    // Copy button
    if (DrawMenuButton(contentX, contentY, 150, 35, "COPY CODE")) {
      SetClipboardText(localAnswerCode);
    }

    // READY button - user clicks after copying code and sending to host
    if (DrawMenuButton(contentX + 170, contentY, 150, 35, "READY")) {
      FinalizeGuestConnection();
    }
    contentY += 50;

    // Instructions
    DrawText("1. Copy the code above", contentX, contentY, FONT_SIZE_SMALL,
             LIGHTGRAY);
    contentY += 20;
    DrawText("2. Send it to the host", contentX, contentY, FONT_SIZE_SMALL,
             LIGHTGRAY);
    contentY += 20;
    DrawText("3. Click READY when done", contentX, contentY, FONT_SIZE_SMALL,
             COLOR_TITLE_GOLD);
  } else if (state == NET_FAILED) {
    DrawText("Connection failed!", contentX, contentY, FONT_SIZE_SMALL, RED);
    DrawText("Please try again.", contentX, contentY + 25, FONT_SIZE_SMALL,
             GRAY);
  }

  // Check if connected - start the game
  if (state == NET_CONNECTED) {
    StartMultiplayerGame();
    currentScreen = SCREEN_GAME;
  }

  // Back button
  if (DrawMenuButton(panelX + (panelWidth - 150) / 2, panelY + panelHeight - 50,
                     150, 35, "CANCEL")) {
    DisconnectNetwork();
    ResetMultiplayer();
    currentScreen = SCREEN_MULTIPLAYER;
  }
}

void HandleJoinInput(void) {
  if (IsKeyPressed(KEY_ESCAPE)) {
    DisconnectNetwork();
    ResetMultiplayer();
    currentScreen = SCREEN_MULTIPLAYER;
  }

  // Handle text input for offer code
  NetworkState state = GetNetworkState();
  if (state == NET_DISCONNECTED) {
    HandleTextInput(inputBuffer, &inputCursor, NET_CODE_MAX_LEN);
  }
}
