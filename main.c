#include "raylib.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
#define BOARD_SIZE 8
#define TILE_SIZE 80
#define BOARD_OFFSET_X 40
#define BOARD_OFFSET_Y 40
#define WINDOW_WIDTH (BOARD_SIZE * TILE_SIZE + BOARD_OFFSET_X * 2)
#define WINDOW_HEIGHT (BOARD_SIZE * TILE_SIZE + BOARD_OFFSET_Y * 2 + 60)

#define SPRITE_SIZE 60
#define SPRITE_COLS 6
#define SPRITE_ROWS 2

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
// Global State
//------------------------------------------------------------------------------
Piece board[BOARD_SIZE][BOARD_SIZE];
PieceColor currentTurn = COLOR_WHITE;
Position selectedPos = {-1, -1};
bool validMoves[BOARD_SIZE][BOARD_SIZE];
GameState gameState = GAME_PLAYING;

// En passant tracking
Position enPassantTarget = {
    -1, -1}; // The square where en passant capture is possible
Position enPassantPawn = {-1, -1}; // The pawn that can be captured en passant

// Promotion
Position promotionPos = {-1, -1};

// Dragging
bool isDragging = false;
Position dragStartPos = {-1, -1};
Vector2 dragOffset = {0, 0};

// Sprite sheet
Texture2D piecesTexture;

//------------------------------------------------------------------------------
// Function Declarations
//------------------------------------------------------------------------------
void InitBoard(void);
void LoadPiecesTexture(void);
void DrawBoard(void);
void DrawPieces(void);
void DrawValidMoves(void);
void DrawUI(void);
void DrawPromotionUI(void);
void DrawGameOverScreen(void);
void HandleInput(void);
void HandlePromotion(void);
void SelectPiece(int row, int col);
void MovePiece(int toRow, int toCol);
void CalculateValidMoves(int row, int col);
void ClearValidMoves(void);
bool IsValidPosition(int row, int col);
bool IsEmpty(int row, int col);
bool IsEnemy(int row, int col, PieceColor color);
bool IsAlly(int row, int col, PieceColor color);
void AddMoveIfValid(int row, int col, PieceColor color);
void CalculatePawnMoves(int row, int col, PieceColor color);
void CalculateRookMoves(int row, int col, PieceColor color);
void CalculateKnightMoves(int row, int col, PieceColor color);
void CalculateBishopMoves(int row, int col, PieceColor color);
void CalculateQueenMoves(int row, int col, PieceColor color);
void CalculateKingMoves(int row, int col, PieceColor color);
bool IsSquareAttacked(int row, int col, PieceColor byColor);
bool IsInCheck(PieceColor color);
bool HasLegalMoves(PieceColor color);
void UpdateGameState(void);
bool WouldBeInCheck(int fromRow, int fromCol, int toRow, int toCol,
                    PieceColor color);
Position FindKing(PieceColor color);
Rectangle GetSpriteRect(PieceType type, PieceColor color);

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main(void) {
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Chess");
  SetTargetFPS(60);

  LoadPiecesTexture();
  InitBoard();

  while (!WindowShouldClose()) {
    if (gameState == GAME_PROMOTING) {
      HandlePromotion();
    } else if (gameState == GAME_CHECKMATE || gameState == GAME_STALEMATE) {
      // Only allow restart in game over state
      if (IsKeyPressed(KEY_R)) {
        InitBoard();
      }
    } else if (gameState == GAME_PLAYING || gameState == GAME_CHECK) {
      HandleInput();
    }

    BeginDrawing();
    ClearBackground((Color){40, 40, 40, 255});

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

//------------------------------------------------------------------------------
// Initialization
//------------------------------------------------------------------------------
void InitBoard(void) {
  // Clear board
  memset(board, 0, sizeof(board));

  // Set up black pieces (top, rows 0-1)
  board[0][0] = (Piece){PIECE_ROOK, COLOR_BLACK, false};
  board[0][1] = (Piece){PIECE_KNIGHT, COLOR_BLACK, false};
  board[0][2] = (Piece){PIECE_BISHOP, COLOR_BLACK, false};
  board[0][3] = (Piece){PIECE_QUEEN, COLOR_BLACK, false};
  board[0][4] = (Piece){PIECE_KING, COLOR_BLACK, false};
  board[0][5] = (Piece){PIECE_BISHOP, COLOR_BLACK, false};
  board[0][6] = (Piece){PIECE_KNIGHT, COLOR_BLACK, false};
  board[0][7] = (Piece){PIECE_ROOK, COLOR_BLACK, false};
  for (int i = 0; i < BOARD_SIZE; i++) {
    board[1][i] = (Piece){PIECE_PAWN, COLOR_BLACK, false};
  }

  // Set up white pieces (bottom, rows 6-7)
  board[7][0] = (Piece){PIECE_ROOK, COLOR_WHITE, false};
  board[7][1] = (Piece){PIECE_KNIGHT, COLOR_WHITE, false};
  board[7][2] = (Piece){PIECE_BISHOP, COLOR_WHITE, false};
  board[7][3] = (Piece){PIECE_QUEEN, COLOR_WHITE, false};
  board[7][4] = (Piece){PIECE_KING, COLOR_WHITE, false};
  board[7][5] = (Piece){PIECE_BISHOP, COLOR_WHITE, false};
  board[7][6] = (Piece){PIECE_KNIGHT, COLOR_WHITE, false};
  board[7][7] = (Piece){PIECE_ROOK, COLOR_WHITE, false};
  for (int i = 0; i < BOARD_SIZE; i++) {
    board[6][i] = (Piece){PIECE_PAWN, COLOR_WHITE, false};
  }

  currentTurn = COLOR_WHITE;
  selectedPos = (Position){-1, -1};
  enPassantTarget = (Position){-1, -1};
  enPassantPawn = (Position){-1, -1};
  gameState = GAME_PLAYING;
  ClearValidMoves();
}

void LoadPiecesTexture(void) { piecesTexture = LoadTexture("ChessPieces.png"); }

//------------------------------------------------------------------------------
// Drawing
//------------------------------------------------------------------------------
void DrawBoard(void) {
  Color lightSquare = (Color){240, 217, 181, 255};
  Color darkSquare = (Color){181, 136, 99, 255};
  Color selectedColor = (Color){255, 255, 0, 100};
  Color checkColor = (Color){255, 0, 0, 150};

  for (int row = 0; row < BOARD_SIZE; row++) {
    for (int col = 0; col < BOARD_SIZE; col++) {
      int x = BOARD_OFFSET_X + col * TILE_SIZE;
      int y = BOARD_OFFSET_Y + row * TILE_SIZE;

      Color color = ((row + col) % 2 == 0) ? lightSquare : darkSquare;
      DrawRectangle(x, y, TILE_SIZE, TILE_SIZE, color);

      // Highlight selected square
      if (row == selectedPos.row && col == selectedPos.col) {
        DrawRectangle(x, y, TILE_SIZE, TILE_SIZE, selectedColor);
      }

      // Highlight king in check
      if ((gameState == GAME_CHECK || gameState == GAME_CHECKMATE) &&
          board[row][col].type == PIECE_KING &&
          board[row][col].color == currentTurn) {
        DrawRectangle(x, y, TILE_SIZE, TILE_SIZE, checkColor);
      }
    }
  }

  // Draw board border
  DrawRectangleLines(BOARD_OFFSET_X - 2, BOARD_OFFSET_Y - 2,
                     BOARD_SIZE * TILE_SIZE + 4, BOARD_SIZE * TILE_SIZE + 4,
                     WHITE);
}

void DrawValidMoves(void) {
  Color validColor = (Color){0, 255, 0, 80};
  Color captureColor = (Color){255, 0, 0, 80};

  for (int row = 0; row < BOARD_SIZE; row++) {
    for (int col = 0; col < BOARD_SIZE; col++) {
      if (validMoves[row][col]) {
        int x = BOARD_OFFSET_X + col * TILE_SIZE;
        int y = BOARD_OFFSET_Y + row * TILE_SIZE;

        if (board[row][col].type != PIECE_NONE ||
            (row == enPassantTarget.row && col == enPassantTarget.col)) {
          DrawRectangle(x, y, TILE_SIZE, TILE_SIZE, captureColor);
        } else {
          DrawCircle(x + TILE_SIZE / 2, y + TILE_SIZE / 2, 15, validColor);
        }
      }
    }
  }
}

Rectangle GetSpriteRect(PieceType type, PieceColor color) {
  // Sprite layout: Row 0 = Black, Row 1 = White
  // Columns: Queen, King, Rook, Knight, Bishop, Pawn
  int col = 0;
  switch (type) {
  case PIECE_QUEEN:
    col = 0;
    break;
  case PIECE_KING:
    col = 1;
    break;
  case PIECE_ROOK:
    col = 2;
    break;
  case PIECE_KNIGHT:
    col = 3;
    break;
  case PIECE_BISHOP:
    col = 4;
    break;
  case PIECE_PAWN:
    col = 5;
    break;
  default:
    break;
  }
  int row = (color == COLOR_WHITE) ? 1 : 0;

  return (Rectangle){col * SPRITE_SIZE, row * SPRITE_SIZE, SPRITE_SIZE,
                     SPRITE_SIZE};
}

void DrawPieces(void) {
  for (int row = 0; row < BOARD_SIZE; row++) {
    for (int col = 0; col < BOARD_SIZE; col++) {
      if (board[row][col].type != PIECE_NONE) {
        // Skip drawing the piece being dragged at its board position
        if (isDragging && row == dragStartPos.row && col == dragStartPos.col) {
          continue;
        }

        Rectangle src =
            GetSpriteRect(board[row][col].type, board[row][col].color);
        int x =
            BOARD_OFFSET_X + col * TILE_SIZE + (TILE_SIZE - SPRITE_SIZE) / 2;
        int y =
            BOARD_OFFSET_Y + row * TILE_SIZE + (TILE_SIZE - SPRITE_SIZE) / 2;
        Rectangle dest = {x, y, SPRITE_SIZE, SPRITE_SIZE};

        DrawTexturePro(piecesTexture, src, dest, (Vector2){0, 0}, 0, WHITE);
      }
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

void DrawUI(void) {
  int y = WINDOW_HEIGHT - 50;

  const char *turnText =
      (currentTurn == COLOR_WHITE) ? "White's Turn" : "Black's Turn";
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

  DrawText(turnText, BOARD_OFFSET_X, y, 24, WHITE);
  DrawText(stateText, BOARD_OFFSET_X + MeasureText(turnText, 24), y, 24,
           stateColor);

  // Restart hint
  if (gameState == GAME_CHECKMATE || gameState == GAME_STALEMATE) {
    DrawText("Press R to restart", WINDOW_WIDTH - 180, y, 20, GRAY);
  }
}

void DrawPromotionUI(void) {
  // Darken background
  DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, (Color){0, 0, 0, 150});

  // Promotion panel - calculate size to fit 4 buttons with spacing
  int buttonSpacing = 10;
  int padding = 20;
  int panelWidth = padding * 2 + 4 * TILE_SIZE + 3 * buttonSpacing;
  int panelHeight = TILE_SIZE + 70;
  int panelX = (WINDOW_WIDTH - panelWidth) / 2;
  int panelY = (WINDOW_HEIGHT - panelHeight) / 2;

  DrawRectangle(panelX, panelY, panelWidth, panelHeight,
                (Color){60, 60, 60, 255});
  DrawRectangleLinesEx((Rectangle){panelX, panelY, panelWidth, panelHeight}, 2,
                       WHITE);

  DrawText("Choose promotion:", panelX + padding, panelY + 12, 20, WHITE);

  PieceType options[] = {PIECE_QUEEN, PIECE_ROOK, PIECE_BISHOP, PIECE_KNIGHT};
  PieceColor color = board[promotionPos.row][promotionPos.col].color;

  for (int i = 0; i < 4; i++) {
    int x = panelX + padding + i * (TILE_SIZE + buttonSpacing);
    int y = panelY + 45;

    DrawRectangle(x, y, TILE_SIZE, TILE_SIZE, (Color){100, 100, 100, 255});
    DrawRectangleLines(x, y, TILE_SIZE, TILE_SIZE, WHITE);

    Rectangle src = GetSpriteRect(options[i], color);
    Rectangle dest = {x + (TILE_SIZE - SPRITE_SIZE) / 2,
                      y + (TILE_SIZE - SPRITE_SIZE) / 2, SPRITE_SIZE,
                      SPRITE_SIZE};
    DrawTexturePro(piecesTexture, src, dest, (Vector2){0, 0}, 0, WHITE);
  }
}

void DrawGameOverScreen(void) {
  // Darken background
  DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, (Color){0, 0, 0, 180});

  // Game over panel
  int panelWidth = 400;
  int panelHeight = 200;
  int panelX = (WINDOW_WIDTH - panelWidth) / 2;
  int panelY = (WINDOW_HEIGHT - panelHeight) / 2;

  // Panel background with border
  DrawRectangle(panelX, panelY, panelWidth, panelHeight,
                (Color){40, 40, 40, 255});
  DrawRectangleLinesEx((Rectangle){panelX, panelY, panelWidth, panelHeight}, 3,
                       WHITE);

  const char *titleText;
  const char *subtitleText;
  Color titleColor;

  if (gameState == GAME_CHECKMATE) {
    titleText = "CHECKMATE!";
    subtitleText = (currentTurn == COLOR_WHITE) ? "Black Wins!" : "White Wins!";
    titleColor = RED;
  } else {
    titleText = "STALEMATE!";
    subtitleText = "It's a Draw!";
    titleColor = GRAY;
  }

  // Title
  int titleWidth = MeasureText(titleText, 48);
  DrawText(titleText, panelX + (panelWidth - titleWidth) / 2, panelY + 30, 48,
           titleColor);

  // Subtitle
  int subtitleWidth = MeasureText(subtitleText, 32);
  DrawText(subtitleText, panelX + (panelWidth - subtitleWidth) / 2, panelY + 90,
           32, WHITE);

  // Restart instruction
  const char *restartText = "Press R to Play Again";
  int restartWidth = MeasureText(restartText, 24);
  DrawText(restartText, panelX + (panelWidth - restartWidth) / 2, panelY + 150,
           24, LIGHTGRAY);
}

//------------------------------------------------------------------------------
// Input Handling
//------------------------------------------------------------------------------
void HandleInput(void) {
  // Restart game
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

        // Calculate offset from piece center
        int pieceX =
            BOARD_OFFSET_X + col * TILE_SIZE + (TILE_SIZE - SPRITE_SIZE) / 2;
        int pieceY =
            BOARD_OFFSET_Y + row * TILE_SIZE + (TILE_SIZE - SPRITE_SIZE) / 2;
        dragOffset = (Vector2){mouse.x - pieceX - SPRITE_SIZE / 2,
                               mouse.y - pieceY - SPRITE_SIZE / 2};
      } else if (selectedPos.row != -1 && validMoves[row][col]) {
        // Click on valid move square (click-to-move still works)
        MovePiece(row, col);
      } else {
        // Click on empty or enemy without selection - deselect
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
        // Valid drop - move the piece
        MovePiece(row, col);
      } else if (row == dragStartPos.row && col == dragStartPos.col) {
        // Dropped on same square - keep selected (click behavior)
        // selectedPos stays as is
      } else {
        // Invalid drop - deselect
        selectedPos = (Position){-1, -1};
        ClearValidMoves();
      }
    } else {
      // Dropped outside board - deselect
      selectedPos = (Position){-1, -1};
      ClearValidMoves();
    }
  }
}

void HandlePromotion(void) {
  if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    Vector2 mouse = GetMousePosition();

    // Use same sizing as DrawPromotionUI
    int buttonSpacing = 10;
    int padding = 20;
    int panelWidth = padding * 2 + 4 * TILE_SIZE + 3 * buttonSpacing;
    int panelHeight = TILE_SIZE + 70;
    int panelX = (WINDOW_WIDTH - panelWidth) / 2;
    int panelY = (WINDOW_HEIGHT - panelHeight) / 2;

    PieceType options[] = {PIECE_QUEEN, PIECE_ROOK, PIECE_BISHOP, PIECE_KNIGHT};

    for (int i = 0; i < 4; i++) {
      int x = panelX + padding + i * (TILE_SIZE + buttonSpacing);
      int y = panelY + 45;

      if (mouse.x >= x && mouse.x < x + TILE_SIZE && mouse.y >= y &&
          mouse.y < y + TILE_SIZE) {
        board[promotionPos.row][promotionPos.col].type = options[i];
        // Switch turns after promotion (was skipped in MovePiece)
        currentTurn = (currentTurn == COLOR_WHITE) ? COLOR_BLACK : COLOR_WHITE;
        gameState = GAME_PLAYING;
        promotionPos = (Position){-1, -1};
        UpdateGameState();
        break;
      }
    }
  }
}

void SelectPiece(int row, int col) {
  if (board[row][col].type != PIECE_NONE &&
      board[row][col].color == currentTurn) {
    selectedPos = (Position){row, col};
    ClearValidMoves();
    CalculateValidMoves(row, col);
  }
}

void MovePiece(int toRow, int toCol) {
  int fromRow = selectedPos.row;
  int fromCol = selectedPos.col;
  Piece piece = board[fromRow][fromCol];

  // Reset en passant
  Position oldEnPassantTarget = enPassantTarget;
  Position oldEnPassantPawn = enPassantPawn;
  enPassantTarget = (Position){-1, -1};
  enPassantPawn = (Position){-1, -1};

  // Handle en passant capture
  if (piece.type == PIECE_PAWN && toRow == oldEnPassantTarget.row &&
      toCol == oldEnPassantTarget.col) {
    board[oldEnPassantPawn.row][oldEnPassantPawn.col] =
        (Piece){PIECE_NONE, COLOR_NONE, false};
  }

  // Handle castling
  if (piece.type == PIECE_KING && abs(toCol - fromCol) == 2) {
    // Move the rook
    if (toCol > fromCol) {
      // Kingside castling
      board[fromRow][5] = board[fromRow][7];
      board[fromRow][5].hasMoved = true;
      board[fromRow][7] = (Piece){PIECE_NONE, COLOR_NONE, false};
    } else {
      // Queenside castling
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

  // Move the piece
  board[toRow][toCol] = piece;
  board[toRow][toCol].hasMoved = true;
  board[fromRow][fromCol] = (Piece){PIECE_NONE, COLOR_NONE, false};

  // Check for pawn promotion
  if (piece.type == PIECE_PAWN) {
    if ((piece.color == COLOR_WHITE && toRow == 0) ||
        (piece.color == COLOR_BLACK && toRow == 7)) {
      promotionPos = (Position){toRow, toCol};
      gameState = GAME_PROMOTING;
      selectedPos = (Position){-1, -1};
      ClearValidMoves();
      return;
    }
  }

  // Switch turns
  currentTurn = (currentTurn == COLOR_WHITE) ? COLOR_BLACK : COLOR_WHITE;
  selectedPos = (Position){-1, -1};
  ClearValidMoves();

  UpdateGameState();
}

//------------------------------------------------------------------------------
// Move Calculation
//------------------------------------------------------------------------------
void ClearValidMoves(void) { memset(validMoves, false, sizeof(validMoves)); }

bool IsValidPosition(int row, int col) {
  return row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE;
}

bool IsEmpty(int row, int col) { return board[row][col].type == PIECE_NONE; }

bool IsEnemy(int row, int col, PieceColor color) {
  return board[row][col].type != PIECE_NONE && board[row][col].color != color;
}

bool IsAlly(int row, int col, PieceColor color) {
  return board[row][col].type != PIECE_NONE && board[row][col].color == color;
}

void AddMoveIfValid(int row, int col, PieceColor color) {
  if (IsValidPosition(row, col) && !IsAlly(row, col, color)) {
    // Check if this move would leave the king in check
    if (!WouldBeInCheck(selectedPos.row, selectedPos.col, row, col, color)) {
      validMoves[row][col] = true;
    }
  }
}

void CalculateValidMoves(int row, int col) {
  Piece piece = board[row][col];

  switch (piece.type) {
  case PIECE_PAWN:
    CalculatePawnMoves(row, col, piece.color);
    break;
  case PIECE_ROOK:
    CalculateRookMoves(row, col, piece.color);
    break;
  case PIECE_KNIGHT:
    CalculateKnightMoves(row, col, piece.color);
    break;
  case PIECE_BISHOP:
    CalculateBishopMoves(row, col, piece.color);
    break;
  case PIECE_QUEEN:
    CalculateQueenMoves(row, col, piece.color);
    break;
  case PIECE_KING:
    CalculateKingMoves(row, col, piece.color);
    break;
  default:
    break;
  }
}

void CalculatePawnMoves(int row, int col, PieceColor color) {
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
  if (IsValidPosition(row + direction, col - 1) &&
      IsEnemy(row + direction, col - 1, color)) {
    AddMoveIfValid(row + direction, col - 1, color);
  }
  if (IsValidPosition(row + direction, col + 1) &&
      IsEnemy(row + direction, col + 1, color)) {
    AddMoveIfValid(row + direction, col + 1, color);
  }

  // En passant
  if (enPassantTarget.row != -1) {
    if (row + direction == enPassantTarget.row &&
        (col - 1 == enPassantTarget.col || col + 1 == enPassantTarget.col)) {
      if (enPassantPawn.row == row && abs(enPassantPawn.col - col) == 1) {
        AddMoveIfValid(enPassantTarget.row, enPassantTarget.col, color);
      }
    }
  }
}

void CalculateRookMoves(int row, int col, PieceColor color) {
  int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

  for (int d = 0; d < 4; d++) {
    for (int i = 1; i < BOARD_SIZE; i++) {
      int r = row + i * directions[d][0];
      int c = col + i * directions[d][1];

      if (!IsValidPosition(r, c))
        break;
      if (IsAlly(r, c, color))
        break;

      AddMoveIfValid(r, c, color);

      if (IsEnemy(r, c, color))
        break;
    }
  }
}

void CalculateKnightMoves(int row, int col, PieceColor color) {
  int moves[8][2] = {{-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
                     {1, -2},  {1, 2},  {2, -1},  {2, 1}};

  for (int i = 0; i < 8; i++) {
    AddMoveIfValid(row + moves[i][0], col + moves[i][1], color);
  }
}

void CalculateBishopMoves(int row, int col, PieceColor color) {
  int directions[4][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};

  for (int d = 0; d < 4; d++) {
    for (int i = 1; i < BOARD_SIZE; i++) {
      int r = row + i * directions[d][0];
      int c = col + i * directions[d][1];

      if (!IsValidPosition(r, c))
        break;
      if (IsAlly(r, c, color))
        break;

      AddMoveIfValid(r, c, color);

      if (IsEnemy(r, c, color))
        break;
    }
  }
}

void CalculateQueenMoves(int row, int col, PieceColor color) {
  CalculateRookMoves(row, col, color);
  CalculateBishopMoves(row, col, color);
}

void CalculateKingMoves(int row, int col, PieceColor color) {
  // Normal moves
  for (int dr = -1; dr <= 1; dr++) {
    for (int dc = -1; dc <= 1; dc++) {
      if (dr == 0 && dc == 0)
        continue;
      AddMoveIfValid(row + dr, col + dc, color);
    }
  }

  // Castling
  if (!board[row][col].hasMoved && !IsInCheck(color)) {
    // Kingside castling
    if (board[row][7].type == PIECE_ROOK && !board[row][7].hasMoved) {
      if (IsEmpty(row, 5) && IsEmpty(row, 6)) {
        if (!IsSquareAttacked(
                row, 5, (color == COLOR_WHITE) ? COLOR_BLACK : COLOR_WHITE) &&
            !IsSquareAttacked(
                row, 6, (color == COLOR_WHITE) ? COLOR_BLACK : COLOR_WHITE)) {
          AddMoveIfValid(row, 6, color);
        }
      }
    }

    // Queenside castling
    if (board[row][0].type == PIECE_ROOK && !board[row][0].hasMoved) {
      if (IsEmpty(row, 1) && IsEmpty(row, 2) && IsEmpty(row, 3)) {
        if (!IsSquareAttacked(
                row, 2, (color == COLOR_WHITE) ? COLOR_BLACK : COLOR_WHITE) &&
            !IsSquareAttacked(
                row, 3, (color == COLOR_WHITE) ? COLOR_BLACK : COLOR_WHITE)) {
          AddMoveIfValid(row, 2, color);
        }
      }
    }
  }
}

//------------------------------------------------------------------------------
// Check Detection
//------------------------------------------------------------------------------
bool IsSquareAttacked(int row, int col, PieceColor byColor) {
  // Check for pawn attacks
  int pawnDir = (byColor == COLOR_WHITE) ? 1 : -1;
  if (IsValidPosition(row + pawnDir, col - 1) &&
      board[row + pawnDir][col - 1].type == PIECE_PAWN &&
      board[row + pawnDir][col - 1].color == byColor) {
    return true;
  }
  if (IsValidPosition(row + pawnDir, col + 1) &&
      board[row + pawnDir][col + 1].type == PIECE_PAWN &&
      board[row + pawnDir][col + 1].color == byColor) {
    return true;
  }

  // Check for knight attacks
  int knightMoves[8][2] = {{-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
                           {1, -2},  {1, 2},  {2, -1},  {2, 1}};
  for (int i = 0; i < 8; i++) {
    int r = row + knightMoves[i][0];
    int c = col + knightMoves[i][1];
    if (IsValidPosition(r, c) && board[r][c].type == PIECE_KNIGHT &&
        board[r][c].color == byColor) {
      return true;
    }
  }

  // Check for king attacks (for adjacent squares)
  for (int dr = -1; dr <= 1; dr++) {
    for (int dc = -1; dc <= 1; dc++) {
      if (dr == 0 && dc == 0)
        continue;
      int r = row + dr;
      int c = col + dc;
      if (IsValidPosition(r, c) && board[r][c].type == PIECE_KING &&
          board[r][c].color == byColor) {
        return true;
      }
    }
  }

  // Check for rook/queen attacks (horizontal/vertical)
  int rookDirs[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
  for (int d = 0; d < 4; d++) {
    for (int i = 1; i < BOARD_SIZE; i++) {
      int r = row + i * rookDirs[d][0];
      int c = col + i * rookDirs[d][1];
      if (!IsValidPosition(r, c))
        break;
      if (board[r][c].type != PIECE_NONE) {
        if (board[r][c].color == byColor && (board[r][c].type == PIECE_ROOK ||
                                             board[r][c].type == PIECE_QUEEN)) {
          return true;
        }
        break;
      }
    }
  }

  // Check for bishop/queen attacks (diagonal)
  int bishopDirs[4][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
  for (int d = 0; d < 4; d++) {
    for (int i = 1; i < BOARD_SIZE; i++) {
      int r = row + i * bishopDirs[d][0];
      int c = col + i * bishopDirs[d][1];
      if (!IsValidPosition(r, c))
        break;
      if (board[r][c].type != PIECE_NONE) {
        if (board[r][c].color == byColor && (board[r][c].type == PIECE_BISHOP ||
                                             board[r][c].type == PIECE_QUEEN)) {
          return true;
        }
        break;
      }
    }
  }

  return false;
}

Position FindKing(PieceColor color) {
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

bool IsInCheck(PieceColor color) {
  Position king = FindKing(color);
  if (king.row == -1)
    return false;

  PieceColor enemyColor = (color == COLOR_WHITE) ? COLOR_BLACK : COLOR_WHITE;
  return IsSquareAttacked(king.row, king.col, enemyColor);
}

bool WouldBeInCheck(int fromRow, int fromCol, int toRow, int toCol,
                    PieceColor color) {
  // Make temporary move
  Piece movingPiece = board[fromRow][fromCol];
  Piece capturedPiece = board[toRow][toCol];
  Piece enPassantCaptured = {PIECE_NONE, COLOR_NONE, false};

  // Handle en passant capture
  bool isEnPassant =
      (movingPiece.type == PIECE_PAWN && toRow == enPassantTarget.row &&
       toCol == enPassantTarget.col);
  if (isEnPassant) {
    enPassantCaptured = board[enPassantPawn.row][enPassantPawn.col];
    board[enPassantPawn.row][enPassantPawn.col] =
        (Piece){PIECE_NONE, COLOR_NONE, false};
  }

  board[toRow][toCol] = movingPiece;
  board[fromRow][fromCol] = (Piece){PIECE_NONE, COLOR_NONE, false};

  bool inCheck = IsInCheck(color);

  // Restore board
  board[fromRow][fromCol] = movingPiece;
  board[toRow][toCol] = capturedPiece;
  if (isEnPassant) {
    board[enPassantPawn.row][enPassantPawn.col] = enPassantCaptured;
  }

  return inCheck;
}

bool HasLegalMoves(PieceColor color) {
  Position savedSelected = selectedPos;

  for (int row = 0; row < BOARD_SIZE; row++) {
    for (int col = 0; col < BOARD_SIZE; col++) {
      if (board[row][col].type != PIECE_NONE &&
          board[row][col].color == color) {
        selectedPos = (Position){row, col};
        ClearValidMoves();
        CalculateValidMoves(row, col);

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
  }

  selectedPos = savedSelected;
  ClearValidMoves();
  return false;
}

void UpdateGameState(void) {
  bool inCheck = IsInCheck(currentTurn);
  bool hasLegalMoves = HasLegalMoves(currentTurn);

  if (inCheck && !hasLegalMoves) {
    gameState = GAME_CHECKMATE;
  } else if (!inCheck && !hasLegalMoves) {
    gameState = GAME_STALEMATE;
  } else if (inCheck) {
    gameState = GAME_CHECK;
  } else {
    gameState = GAME_PLAYING;
  }
}
