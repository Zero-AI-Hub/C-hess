@echo off
REM Download Mesa3D software OpenGL renderer for Windows
REM This allows the game to run without GPU drivers

echo Downloading Mesa3D opengl32.dll...
echo This provides software OpenGL rendering for systems without GPU drivers.
echo.

REM Try PowerShell download (available on all modern Windows)
powershell -Command "& {[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; Invoke-WebRequest -Uri 'https://downloads.fdossena.com/geth.php?r=mesa64-latest' -OutFile 'opengl32.dll'}" 2>nul

if exist opengl32.dll (
    echo.
    echo SUCCESS! Mesa3D opengl32.dll downloaded.
    echo You can now run chess.exe with software rendering.
    echo.
) else (
    echo.
    echo Download failed. Please download manually:
    echo   1. Go to: https://fdossena.com/?p=mesa/index.frag
    echo   2. Download the 64-bit version
    echo   3. Extract opengl32.dll to this folder
    echo.
)

pause
