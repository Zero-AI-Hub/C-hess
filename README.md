# Chess Game

A complete local multiplayer chess game written in C using the raylib library.

## Features

- **Full Chess Rules**: All standard chess rules implemented
- **Piece Movement**: All pieces move according to standard chess rules
- **Special Moves**:
  - Castling (both kingside and queenside)
  - En passant capture
  - Pawn promotion with piece selection
- **Game State Detection**:
  - Check detection with visual highlight
  - Checkmate detection
  - Stalemate detection
- **Visual Feedback**:
  - Highlighted valid moves
  - Selected piece highlight
  - Turn indicator
  - Game state display

## Prerequisites

- **GCC** (C compiler)
- **Git** (to download raylib automatically)
- **X11 development libraries** (for Linux)

### Installing dependencies on Ubuntu/Debian

```bash
sudo apt install build-essential git libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libgl1-mesa-dev
```

### Installing dependencies on Arch Linux

```bash
sudo pacman -S base-devel git libx11 libxrandr libxinerama libxcursor libxi mesa
```

## Building

Simply run:

```bash
make
```

This will:
1. Automatically download raylib (if not present)
2. Build raylib as a static library
3. Compile the chess game

The first build takes longer due to raylib compilation. Subsequent builds are faster.

### Makefile targets

| Target | Description |
|--------|-------------|
| `make` | Build raylib (if needed) and the game |
| `make clean` | Remove the chess executable |
| `make clean-raylib` | Remove the raylib directory |
| `make clean-all` | Remove both the executable and raylib |

## How to Play

1. Run the game:
   ```bash
   ./chess
   ```

2. **Controls**:
   - **Left Click** on a piece to select it
   - **Left Click** on a highlighted square to move the selected piece
   - **Left Click** on another piece of your color to select it instead
   - **R** to restart the game

3. **Game Rules**:
   - White moves first
   - Players alternate turns
   - Click on your pieces to see valid moves (green circles)
   - Capture moves are shown with a red highlight
   - When in check, the king is highlighted in red
   - The game ends on checkmate or stalemate

4. **Special Moves**:
   - **Castling**: Click on the king, then on the square two squares away (if castling is legal)
   - **En Passant**: Immediately after an opponent pawn moves two squares, you can capture it by moving diagonally behind it
   - **Pawn Promotion**: When a pawn reaches the opposite end, a menu appears to select the new piece

## Project Structure

```
chess-c/
├── main.c          # Game source code
├── Makefile        # Build configuration
├── ChessPieces.png # Sprite sheet for chess pieces
├── README.md       # This file
└── raylib/         # (auto-downloaded) raylib library
```

## License

This project is provided as-is for educational and personal use.
