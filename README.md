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
- **Git** (to download raylib and libjuice automatically)
- **Make** (build automation tool)

---

## Building on Linux

### Installing dependencies on Ubuntu/Debian

```bash
sudo apt install build-essential git libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libgl1-mesa-dev
```

### Installing dependencies on Arch Linux

```bash
sudo pacman -S base-devel git libx11 libxrandr libxinerama libxcursor libxi mesa
```

### Building

```bash
make
```

### Running

```bash
./chess
```

---

## Building on Windows

### Option 1: Using MSYS2 with MinGW (Recommended)

1. **Install MSYS2** from [https://www.msys2.org/](https://www.msys2.org/)

2. **Open MSYS2 MinGW 64-bit terminal** and install dependencies:
   ```bash
   pacman -Syu
   pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-make git
   ```

3. **Clone the project** (or copy it to your MSYS2 home directory):
   ```bash
   git clone <repository-url>
   cd chess-c
   ```

4. **Build the game**:
   ```bash
   mingw32-make
   ```

5. **Run the game**:
   ```bash
   ./chess.exe
   ```

### Option 2: Using WSL (Windows Subsystem for Linux)

1. **Install WSL** with Ubuntu from Microsoft Store

2. **Install dependencies** (same as Linux):
   ```bash
   sudo apt install build-essential git libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libgl1-mesa-dev
   ```

3. **Build and run** (same as Linux):
   ```bash
   make
   ./chess
   ```

> **Note**: WSL2 with GUI support (WSLg) is required to run graphical applications.

---

## Build System

The Makefile automatically:
1. Detects your operating system (Linux, Windows, macOS)
2. Downloads raylib (if not present)
3. Downloads libjuice (if not present)
4. Builds both libraries as static libraries
5. Compiles and links the chess game

The first build takes longer due to library compilation. Subsequent builds are faster.

### Makefile targets

| Target | Description |
|--------|-------------|
| `make` | Build raylib, libjuice (if needed) and the game |
| `make clean` | Remove the chess executable and object files |
| `make clean-raylib` | Remove the raylib directory |
| `make clean-libjuice` | Remove the libjuice directory |
| `make clean-all` | Remove executable, object files, and all libraries |
| `make help` | Show available targets and detected platform |

---

## How to Play

1. **Run the game**:
   - Linux: `./chess`
   - Windows: `./chess.exe` or double-click `chess.exe`

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

---

## Project Structure

```
chess-c/
├── main.c          # Entry point and game loop
├── board.c/h       # Board state management
├── moves.c/h       # Move generation and validation
├── check.c/h       # Check, checkmate, stalemate detection
├── ui.c/h          # User interface rendering
├── menu.c/h        # Menu system
├── history.c/h     # Move history and notation
├── clock.c/h       # Chess clock functionality
├── network.c/h     # Network/multiplayer code
├── multiplayer.c/h # Multiplayer game logic
├── types.h         # Shared type definitions
├── constants.c     # Game constants
├── Makefile        # Cross-platform build configuration
├── ChessPieces.png # Sprite sheet for chess pieces
├── README.md       # This file
├── MULTIPLAYER.md  # Multiplayer documentation
├── raylib/         # (auto-downloaded) raylib library
└── libjuice/       # (auto-downloaded) libjuice library
```

---

## Troubleshooting

### Linux
- **Missing libraries**: Ensure all X11 development libraries are installed
- **OpenGL errors**: Update your graphics drivers

### Windows
- **Command not found**: Make sure MinGW binaries are in your PATH
- **Build errors**: Use `mingw32-make` instead of `make` in MSYS2 MinGW terminal
- **Missing DLLs**: Build with static linking (default) or copy required DLLs to the executable directory

---

## License

This project is provided as-is for educational and personal use.
