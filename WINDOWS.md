# Building on Windows (PowerShell)

This branch includes PowerShell scripts that allow you to build the chess game directly from Windows **without needing MSYS2**.

## Quick Start

1. **Open PowerShell** in this directory

2. **Run the build script:**
   ```powershell
   powershell -ExecutionPolicy Bypass -File build-windows.ps1
   ```

3. **Play!**
   ```powershell
   .\chess.exe
   ```

## What the script does

The `build-windows.ps1` script automatically:

1. **Downloads MinGW** (~60MB) from [nuwen.net](https://nuwen.net/mingw.html) if not already installed
   - Installs to `%USERPROFILE%\MinGW`
   - Includes GCC 13.2.0, make, and other tools
   
2. **Clones raylib** from GitHub and compiles it

3. **Clones libjuice** from GitHub and compiles it

4. **Compiles and links `chess.exe`**

## Troubleshooting

### OpenGL errors

If you see an error like *"The driver does not appear to support OpenGL"*, your system lacks GPU drivers. Run:

```powershell
powershell -ExecutionPolicy Bypass -File download-mesa.ps1
```

This downloads a software OpenGL renderer that works everywhere.

### Execution Policy errors

If PowerShell blocks the scripts, use this command instead:

```powershell
powershell -ExecutionPolicy Bypass -File build-windows.ps1
```

### Clean build

To rebuild everything from scratch:

```powershell
Remove-Item *.o, chess.exe -Force
.\build-windows.ps1
```

To also rebuild the libraries:

```powershell
Remove-Item raylib, libjuice -Recurse -Force
.\build-windows.ps1
```

## Files added in this branch

| File | Description |
|------|-------------|
| `build-windows.ps1` | Main build script - downloads tools, compiles everything |
| `download-mesa.ps1` | Downloads Mesa3D software renderer for OpenGL issues |
| `WINDOWS.md` | This file |

## Original build method

The original Makefile still works if you have MSYS2 installed:

```bash
# In MSYS2 MinGW 64-bit terminal
mingw32-make
./chess.exe
```

See the main [README.md](README.md) for full documentation.
