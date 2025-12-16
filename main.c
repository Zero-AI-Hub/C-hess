/**
 * Chess Game - A complete chess implementation using Raylib
 *
 * Features:
 * - Full chess rules including castling, en passant, and pawn promotion
 * - Click-to-move and drag-and-drop piece movement
 * - Check, checkmate, and stalemate detection
 * - Visual highlighting for valid moves and check state
 */

#include "raylib.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

//==============================================================================
// CONSTANTS
//==============================================================================

// Board layout
#define BOARD_SIZE 8
#define TILE_SIZE 80
#define BOARD_OFFSET_X 40
#define BOARD_OFFSET_Y 40
#define WINDOW_WIDTH (BOARD_SIZE * TILE_SIZE + BOARD_OFFSET_X * 2)
#define WINDOW_HEIGHT (BOARD_SIZE * TILE_SIZE + BOARD_OFFSET_Y * 2 + 60)

// Sprite configuration
#define SPRITE_SIZE 60
#define SPRITE_COLS 6
#define SPRITE_ROWS 2

// UI constants
#define VALID_MOVE_RADIUS 15
#define PANEL_PADDING 20
#define BUTTON_SPACING 10
#define FONT_SIZE_SMALL 20
#define FONT_SIZE_MEDIUM 24
#define FONT_SIZE_LARGE 32
#define FONT_SIZE_TITLE 48

// Helper macro for getting opponent color
#define OPPONENT_COLOR(c) ((c) == COLOR_WHITE ? COLOR_BLACK : COLOR_WHITE)

//==============================================================================
// COLOR PALETTE
//==============================================================================

static const Color COLOR_LIGHT_SQUARE = {240, 217, 181, 255};
static const Color COLOR_DARK_SQUARE = {181, 136, 99, 255};
static const Color COLOR_SELECTED = {255, 255, 0, 100};
static const Color COLOR_CHECK_HIGHLIGHT = {255, 0, 0, 150};
static const Color COLOR_VALID_MOVE = {0, 255, 0, 80};
static const Color COLOR_CAPTURE = {255, 0, 0, 80};
static const Color COLOR_BACKGROUND = {40, 40, 40, 255};
static const Color COLOR_OVERLAY_DARK = {0, 0, 0, 150};
static const Color COLOR_OVERLAY_DARKER = {0, 0, 0, 180};
static const Color COLOR_PANEL_BG = {60, 60, 60, 255};
static const Color COLOR_BUTTON_BG = {100, 100, 100, 255};

//==============================================================================
// MOVEMENT PATTERNS (shared across multiple functions)
//==============================================================================

static const int ROOK_DIRECTIONS[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
static const int BISHOP_DIRECTIONS[4][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
static const int KNIGHT_MOVES[8][2] = {{-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
                                       {1, -2},  {1, 2},  {2, -1},  {2, 1}};

//==============================================================================
// TYPES
//==============================================================================

typedef enum {
  PIECE_NONE = 0,
  PIECE_KING,
  PIECE_QUEEN,
  PIECE_BISHOP,
  PIECE_KNIGHT,
  PIECE_ROOK,
  PIECE_PAWN
} PieceType;

typedef enum { COLOR_NONE = 0, COLOR_WHITE, COLOR_BLACK } PieceColor;

typedef struct {
  PieceType type;
  PieceColor color;
  bool hasMoved;
} Piece;

typedef struct {
  int row;
  int col;
} Position;

typedef enum {
  GAME_PLAYING,
  GAME_CHECK,
  GAME_CHECKMATE,
  GAME_STALEMATE,
  GAME_PROMOTING
} GameState;

// Helper struct for UI panel layouts
typedef struct {
  int x, y, width, height;
} PanelLayout;

//==============================================================================
// GLOBAL STATE
//==============================================================================

static Piece board[BOARD_SIZE][BOARD_SIZE];
static PieceColor currentTurn = COLOR_WHITE;
static Position selectedPos = {-1, -1};
static bool validMoves[BOARD_SIZE][BOARD_SIZE];
static GameState gameState = GAME_PLAYING;

// En passant tracking:
// - enPassantTarget: the "ghost" square where pawn can be captured
// - enPassantPawn: the actual pawn that moved two squares
static Position enPassantTarget = {-1, -1};
static Position enPassantPawn = {-1, -1};

// Pawn promotion
static Position promotionPos = {-1, -1};

// Drag-and-drop state
static bool isDragging = false;
static Position dragStartPos = {-1, -1};
static Vector2 dragOffset = {0, 0};

// Graphics
static Texture2D piecesTexture;

//==============================================================================
// FUNCTION DECLARATIONS
//==============================================================================

// Initialization
static void InitBoard(void);
static void LoadPiecesTexture(void);

// Drawing
static void DrawBoard(void);
static void DrawPieces(void);
static void DrawValidMoves(void);
static void DrawUI(void);
static void DrawPromotionUI(void);
static void DrawGameOverScreen(void);
static Rectangle GetSpriteRect(PieceType type, PieceColor color);

// Input handling
static void HandleInput(void);
static void HandlePromotion(void);
static void MovePiece(int toRow, int toCol);

// Move calculation
static void CalculateValidMoves(int row, int col);
static void ClearValidMoves(void);
static void CalculatePawnMoves(int row, int col, PieceColor color);
static void CalculateSlidingMoves(int row, int col, PieceColor color,
                                  const int dirs[4][2]);
static void CalculateKnightMoves(int row, int col, PieceColor color);
static void CalculateKingMoves(int row, int col, PieceColor color);

// Board queries
static bool IsValidPosition(int row, int col);
static bool IsEmpty(int row, int col);
static bool IsEnemy(int row, int col, PieceColor color);
static bool IsAlly(int row, int col, PieceColor color);
static void AddMoveIfValid(int row, int col, PieceColor color);

// Check detection
static bool IsSquareAttacked(int row, int col, PieceColor byColor);
static bool IsInCheck(PieceColor color);
static bool WouldBeInCheck(int fromRow, int fromCol, int toRow, int toCol,
                           PieceColor color);
static Position FindKing(PieceColor color);
static bool HasLegalMoves(PieceColor color);
static void UpdateGameState(void);

// UI helpers
static PanelLayout GetPromotionPanelLayout(void);

//==============================================================================
// MAIN
//==============================================================================

int main(void) {
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Chess");
  SetTargetFPS(60);

  LoadPiecesTexture();
  InitBoard();

  while (!WindowShouldClose()) {
    // Handle game state-specific input
    if (gameState == GAME_PROMOTING) {
      HandlePromotion();
    } else if (gameState == GAME_CHECKMATE || gameState == GAME_STALEMATE) {
      if (IsKeyPressed(KEY_R)) {
        InitBoard();
      }
    } else {
      HandleInput();
    }

    // Render
    BeginDrawing();
    ClearBackground(COLOR_BACKGROUND);

    DrawBoard();
    DrawValidMoves();
    DrawPieces();
    DrawUI();

    if (gameState == GAME_PROMOTING) {
      DrawPromotionUI();
    } else if (gameState == GAME_CHECKMATE || gameState == GAME_STALEMATE) {
      DrawGameOverScreen();
    }

    EndDrawing();
  }

  UnloadTexture(piecesTexture);
  CloseWindow();
  return 0;
}

//==============================================================================
// INITIALIZATION
//==============================================================================

/**
 * Initializes the board with pieces in their starting positions.
 * White pieces on rows 6-7, black pieces on rows 0-1.
 */
static void InitBoard(void) {
  memset(board, 0, sizeof(board));

  // Piece types for back row: Rook, Knight, Bishop, Queen, King, Bishop,
  // Knight, Rook
  static const PieceType backRow[8] = {PIECE_ROOK,   PIECE_KNIGHT, PIECE_BISHOP,
                                       PIECE_QUEEN,  PIECE_KING,   PIECE_BISHOP,
                                       PIECE_KNIGHT, PIECE_ROOK};

  for (int i = 0; i < BOARD_SIZE; i++) {
    // Black pieces (top)
    board[0][i] = (Piece){backRow[i], COLOR_BLACK, false};
    board[1][i] = (Piece){PIECE_PAWN, COLOR_BLACK, false};
    // White pieces (bottom)
    board[6][i] = (Piece){PIECE_PAWN, COLOR_WHITE, false};
    board[7][i] = (Piece){backRow[i], COLOR_WHITE, false};
  }

  // Reset game state
  currentTurn = COLOR_WHITE;
  selectedPos = (Position){-1, -1};
  enPassantTarget = (Position){-1, -1};
  enPassantPawn = (Position){-1, -1};
  gameState = GAME_PLAYING;
  ClearValidMoves();
}

static void LoadPiecesTexture(void) {
  piecesTexture = LoadTexture("ChessPieces.png");
}

//==============================================================================
// DRAWING
//==============================================================================

static void DrawBoard(void) {
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

static void DrawValidMoves(void) {
  for (int row = 0; row < BOARD_SIZE; row++) {
    for (int col = 0; col < BOARD_SIZE; col++) {
      if (!validMoves[row][col])
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

/**
 * Returns the sprite rectangle for a given piece type and color.
 * Sprite sheet layout: Row 0 = Black, Row 1 = White
 * Columns: Queen, King, Rook, Knight, Bishop, Pawn
 */
static Rectangle GetSpriteRect(PieceType type, PieceColor color) {
  // Map piece type enum to sprite column index
  static const int SPRITE_COL[7] = {0, 1, 0, 4, 3, 2, 5};
  int col = SPRITE_COL[type];
  int row = (color == COLOR_WHITE) ? 1 : 0;
  return (Rectangle){col * SPRITE_SIZE, row * SPRITE_SIZE, SPRITE_SIZE,
                     SPRITE_SIZE};
}

static void DrawPieces(void) {
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

static void DrawUI(void) {
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
    DrawText("Press R to restart", WINDOW_WIDTH - 180, y, FONT_SIZE_SMALL,
             GRAY);
  }
}

/**
 * Helper function to calculate promotion panel position and size.
 * Used by both DrawPromotionUI and HandlePromotion for consistency.
 */
static PanelLayout GetPromotionPanelLayout(void) {
  int panelWidth = PANEL_PADDING * 2 + 4 * TILE_SIZE + 3 * BUTTON_SPACING;
  int panelHeight = TILE_SIZE + 70;
  return (PanelLayout){(WINDOW_WIDTH - panelWidth) / 2,
                       (WINDOW_HEIGHT - panelHeight) / 2, panelWidth,
                       panelHeight};
}

static void DrawPromotionUI(void) {
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

static void DrawGameOverScreen(void) {
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

static void HandleInput(void) {
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
      } else if (selectedPos.row != -1 && validMoves[row][col]) {
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
      if (validMoves[row][col]) {
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

static void HandlePromotion(void) {
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
      board[promotionPos.row][promotionPos.col].type = options[i];
      currentTurn = OPPONENT_COLOR(currentTurn);
      gameState = GAME_PLAYING;
      promotionPos = (Position){-1, -1};
      UpdateGameState();
      break;
    }
  }
}

// Note: SelectPiece was merged into HandleInput for drag-and-drop support

static void MovePiece(int toRow, int toCol) {
  int fromRow = selectedPos.row;
  int fromCol = selectedPos.col;
  Piece piece = board[fromRow][fromCol];

  // Save and reset en passant state
  Position oldEnPassantTarget = enPassantTarget;
  Position oldEnPassantPawn = enPassantPawn;
  enPassantTarget = (Position){-1, -1};
  enPassantPawn = (Position){-1, -1};

  // Handle en passant capture: remove the captured pawn
  if (piece.type == PIECE_PAWN && toRow == oldEnPassantTarget.row &&
      toCol == oldEnPassantTarget.col) {
    board[oldEnPassantPawn.row][oldEnPassantPawn.col] =
        (Piece){PIECE_NONE, COLOR_NONE, false};
  }

  // Handle castling: move the rook alongside the king
  if (piece.type == PIECE_KING && abs(toCol - fromCol) == 2) {
    if (toCol > fromCol) {
      // Kingside castling - move rook from h-file to f-file
      board[fromRow][5] = board[fromRow][7];
      board[fromRow][5].hasMoved = true;
      board[fromRow][7] = (Piece){PIECE_NONE, COLOR_NONE, false};
    } else {
      // Queenside castling - move rook from a-file to d-file
      board[fromRow][3] = board[fromRow][0];
      board[fromRow][3].hasMoved = true;
      board[fromRow][0] = (Piece){PIECE_NONE, COLOR_NONE, false};
    }
  }

  // Set en passant target if pawn moves two squares
  if (piece.type == PIECE_PAWN && abs(toRow - fromRow) == 2) {
    enPassantTarget = (Position){(fromRow + toRow) / 2, fromCol};
    enPassantPawn = (Position){toRow, toCol};
  }

  // Execute the move
  board[toRow][toCol] = piece;
  board[toRow][toCol].hasMoved = true;
  board[fromRow][fromCol] = (Piece){PIECE_NONE, COLOR_NONE, false};

  // Check for pawn promotion
  if (piece.type == PIECE_PAWN &&
      ((piece.color == COLOR_WHITE && toRow == 0) ||
       (piece.color == COLOR_BLACK && toRow == 7))) {
    promotionPos = (Position){toRow, toCol};
    gameState = GAME_PROMOTING;
    selectedPos = (Position){-1, -1};
    ClearValidMoves();
    return;
  }

  // Switch turns and update game state
  currentTurn = OPPONENT_COLOR(currentTurn);
  selectedPos = (Position){-1, -1};
  ClearValidMoves();
  UpdateGameState();
}

//==============================================================================
// MOVE CALCULATION
//==============================================================================

static void ClearValidMoves(void) {
  memset(validMoves, false, sizeof(validMoves));
}

static bool IsValidPosition(int row, int col) {
  return row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE;
}

static bool IsEmpty(int row, int col) {
  return board[row][col].type == PIECE_NONE;
}

static bool IsEnemy(int row, int col, PieceColor color) {
  return board[row][col].type != PIECE_NONE && board[row][col].color != color;
}

static bool IsAlly(int row, int col, PieceColor color) {
  return board[row][col].type != PIECE_NONE && board[row][col].color == color;
}

/**
 * Adds a move to validMoves if it's legal (doesn't leave king in check).
 */
static void AddMoveIfValid(int row, int col, PieceColor color) {
  if (IsValidPosition(row, col) && !IsAlly(row, col, color)) {
    if (!WouldBeInCheck(selectedPos.row, selectedPos.col, row, col, color)) {
      validMoves[row][col] = true;
    }
  }
}

static void CalculateValidMoves(int row, int col) {
  Piece piece = board[row][col];

  switch (piece.type) {
  case PIECE_PAWN:
    CalculatePawnMoves(row, col, piece.color);
    break;
  case PIECE_ROOK:
    CalculateSlidingMoves(row, col, piece.color, ROOK_DIRECTIONS);
    break;
  case PIECE_KNIGHT:
    CalculateKnightMoves(row, col, piece.color);
    break;
  case PIECE_BISHOP:
    CalculateSlidingMoves(row, col, piece.color, BISHOP_DIRECTIONS);
    break;
  case PIECE_QUEEN:
    // Queen = Rook + Bishop movement
    CalculateSlidingMoves(row, col, piece.color, ROOK_DIRECTIONS);
    CalculateSlidingMoves(row, col, piece.color, BISHOP_DIRECTIONS);
    break;
  case PIECE_KING:
    CalculateKingMoves(row, col, piece.color);
    break;
  default:
    break;
  }
}

static void CalculatePawnMoves(int row, int col, PieceColor color) {
  int direction = (color == COLOR_WHITE) ? -1 : 1;
  int startRow = (color == COLOR_WHITE) ? 6 : 1;

  // Forward one square
  if (IsValidPosition(row + direction, col) && IsEmpty(row + direction, col)) {
    AddMoveIfValid(row + direction, col, color);

    // Forward two squares from starting position
    if (row == startRow && IsEmpty(row + 2 * direction, col)) {
      AddMoveIfValid(row + 2 * direction, col, color);
    }
  }

  // Diagonal captures
  for (int dc = -1; dc <= 1; dc += 2) {
    if (IsValidPosition(row + direction, col + dc) &&
        IsEnemy(row + direction, col + dc, color)) {
      AddMoveIfValid(row + direction, col + dc, color);
    }
  }

  // En passant capture
  if (enPassantTarget.row != -1 && row + direction == enPassantTarget.row &&
      abs(col - enPassantTarget.col) == 1 && enPassantPawn.row == row &&
      abs(enPassantPawn.col - col) == 1) {
    AddMoveIfValid(enPassantTarget.row, enPassantTarget.col, color);
  }
}

/**
 * Calculates moves for sliding pieces (rook, bishop, queen).
 * Moves in each direction until blocked by edge, ally, or enemy (captured).
 */
static void CalculateSlidingMoves(int row, int col, PieceColor color,
                                  const int dirs[4][2]) {
  for (int d = 0; d < 4; d++) {
    for (int i = 1; i < BOARD_SIZE; i++) {
      int targetRow = row + i * dirs[d][0];
      int targetCol = col + i * dirs[d][1];

      if (!IsValidPosition(targetRow, targetCol))
        break;
      if (IsAlly(targetRow, targetCol, color))
        break;

      AddMoveIfValid(targetRow, targetCol, color);

      if (IsEnemy(targetRow, targetCol, color))
        break;
    }
  }
}

static void CalculateKnightMoves(int row, int col, PieceColor color) {
  for (int i = 0; i < 8; i++) {
    AddMoveIfValid(row + KNIGHT_MOVES[i][0], col + KNIGHT_MOVES[i][1], color);
  }
}

static void CalculateKingMoves(int row, int col, PieceColor color) {
  // Normal moves (8 adjacent squares)
  for (int dr = -1; dr <= 1; dr++) {
    for (int dc = -1; dc <= 1; dc++) {
      if (dr == 0 && dc == 0)
        continue;
      AddMoveIfValid(row + dr, col + dc, color);
    }
  }

  // Castling: king and rook haven't moved, not in check, squares clear and safe
  if (board[row][col].hasMoved || IsInCheck(color))
    return;

  PieceColor enemy = OPPONENT_COLOR(color);

  // Kingside castling (O-O)
  if (board[row][7].type == PIECE_ROOK && !board[row][7].hasMoved &&
      IsEmpty(row, 5) && IsEmpty(row, 6) && !IsSquareAttacked(row, 5, enemy) &&
      !IsSquareAttacked(row, 6, enemy)) {
    AddMoveIfValid(row, 6, color);
  }

  // Queenside castling (O-O-O)
  if (board[row][0].type == PIECE_ROOK && !board[row][0].hasMoved &&
      IsEmpty(row, 1) && IsEmpty(row, 2) && IsEmpty(row, 3) &&
      !IsSquareAttacked(row, 2, enemy) && !IsSquareAttacked(row, 3, enemy)) {
    AddMoveIfValid(row, 2, color);
  }
}

//==============================================================================
// CHECK DETECTION
//==============================================================================

/**
 * Determines if a square is under attack by any piece of the specified color.
 * Used for check detection and castling validation.
 */
static bool IsSquareAttacked(int row, int col, PieceColor byColor) {
  // Pawn attacks (pawns attack diagonally)
  int pawnDir = (byColor == COLOR_WHITE) ? 1 : -1;
  for (int dc = -1; dc <= 1; dc += 2) {
    int pawnRow = row + pawnDir;
    int pawnCol = col + dc;
    if (IsValidPosition(pawnRow, pawnCol) &&
        board[pawnRow][pawnCol].type == PIECE_PAWN &&
        board[pawnRow][pawnCol].color == byColor) {
      return true;
    }
  }

  // Knight attacks
  for (int i = 0; i < 8; i++) {
    int targetRow = row + KNIGHT_MOVES[i][0];
    int targetCol = col + KNIGHT_MOVES[i][1];
    if (IsValidPosition(targetRow, targetCol) &&
        board[targetRow][targetCol].type == PIECE_KNIGHT &&
        board[targetRow][targetCol].color == byColor) {
      return true;
    }
  }

  // King attacks (adjacent squares)
  for (int dr = -1; dr <= 1; dr++) {
    for (int dc = -1; dc <= 1; dc++) {
      if (dr == 0 && dc == 0)
        continue;
      int targetRow = row + dr;
      int targetCol = col + dc;
      if (IsValidPosition(targetRow, targetCol) &&
          board[targetRow][targetCol].type == PIECE_KING &&
          board[targetRow][targetCol].color == byColor) {
        return true;
      }
    }
  }

  // Rook/Queen attacks (horizontal/vertical)
  for (int d = 0; d < 4; d++) {
    for (int i = 1; i < BOARD_SIZE; i++) {
      int targetRow = row + i * ROOK_DIRECTIONS[d][0];
      int targetCol = col + i * ROOK_DIRECTIONS[d][1];
      if (!IsValidPosition(targetRow, targetCol))
        break;
      if (board[targetRow][targetCol].type != PIECE_NONE) {
        if (board[targetRow][targetCol].color == byColor &&
            (board[targetRow][targetCol].type == PIECE_ROOK ||
             board[targetRow][targetCol].type == PIECE_QUEEN)) {
          return true;
        }
        break;
      }
    }
  }

  // Bishop/Queen attacks (diagonal)
  for (int d = 0; d < 4; d++) {
    for (int i = 1; i < BOARD_SIZE; i++) {
      int targetRow = row + i * BISHOP_DIRECTIONS[d][0];
      int targetCol = col + i * BISHOP_DIRECTIONS[d][1];
      if (!IsValidPosition(targetRow, targetCol))
        break;
      if (board[targetRow][targetCol].type != PIECE_NONE) {
        if (board[targetRow][targetCol].color == byColor &&
            (board[targetRow][targetCol].type == PIECE_BISHOP ||
             board[targetRow][targetCol].type == PIECE_QUEEN)) {
          return true;
        }
        break;
      }
    }
  }

  return false;
}

static Position FindKing(PieceColor color) {
  for (int row = 0; row < BOARD_SIZE; row++) {
    for (int col = 0; col < BOARD_SIZE; col++) {
      if (board[row][col].type == PIECE_KING &&
          board[row][col].color == color) {
        return (Position){row, col};
      }
    }
  }
  return (Position){-1, -1};
}

static bool IsInCheck(PieceColor color) {
  Position king = FindKing(color);
  if (king.row == -1)
    return false;
  return IsSquareAttacked(king.row, king.col, OPPONENT_COLOR(color));
}

/**
 * Simulates a move temporarily to check if it would leave the king in check.
 * Essential for filtering out illegal moves that expose the king.
 */
static bool WouldBeInCheck(int fromRow, int fromCol, int toRow, int toCol,
                           PieceColor color) {
  // Save board state
  Piece movingPiece = board[fromRow][fromCol];
  Piece capturedPiece = board[toRow][toCol];
  Piece enPassantCaptured = {PIECE_NONE, COLOR_NONE, false};

  // Handle en passant capture simulation
  bool isEnPassant =
      (movingPiece.type == PIECE_PAWN && toRow == enPassantTarget.row &&
       toCol == enPassantTarget.col);
  if (isEnPassant) {
    enPassantCaptured = board[enPassantPawn.row][enPassantPawn.col];
    board[enPassantPawn.row][enPassantPawn.col] =
        (Piece){PIECE_NONE, COLOR_NONE, false};
  }

  // Make temporary move
  board[toRow][toCol] = movingPiece;
  board[fromRow][fromCol] = (Piece){PIECE_NONE, COLOR_NONE, false};

  bool inCheck = IsInCheck(color);

  // Restore board state
  board[fromRow][fromCol] = movingPiece;
  board[toRow][toCol] = capturedPiece;
  if (isEnPassant) {
    board[enPassantPawn.row][enPassantPawn.col] = enPassantCaptured;
  }

  return inCheck;
}

/**
 * Checks if the specified color has any legal moves available.
 * Used for checkmate/stalemate detection.
 */
static bool HasLegalMoves(PieceColor color) {
  Position savedSelected = selectedPos;

  for (int row = 0; row < BOARD_SIZE; row++) {
    for (int col = 0; col < BOARD_SIZE; col++) {
      if (board[row][col].type == PIECE_NONE || board[row][col].color != color)
        continue;

      selectedPos = (Position){row, col};
      ClearValidMoves();
      CalculateValidMoves(row, col);

      // Check if any valid move exists
      for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
          if (validMoves[r][c]) {
            selectedPos = savedSelected;
            ClearValidMoves();
            return true;
          }
        }
      }
    }
  }

  selectedPos = savedSelected;
  ClearValidMoves();
  return false;
}

/**
 * Updates gameState based on current board position.
 * Called after each move to detect check, checkmate, or stalemate.
 */
static void UpdateGameState(void) {
  bool inCheck = IsInCheck(currentTurn);
  bool hasLegalMoves = HasLegalMoves(currentTurn);

  if (!hasLegalMoves) {
    gameState = inCheck ? GAME_CHECKMATE : GAME_STALEMATE;
  } else {
    gameState = inCheck ? GAME_CHECK : GAME_PLAYING;
  }
}
