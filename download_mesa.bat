@echo off
REM Download Mesa3D software OpenGL renderer for Windows
REM This allows the game to run without GPU drivers

echo ============================================
echo  Mesa3D Software OpenGL Renderer Installer
echo ============================================
echo.
echo This will download opengl32.dll which provides
echo software OpenGL rendering for systems without
echo working GPU drivers.
echo.

REM Check if already exists
if exist opengl32.dll (
    echo opengl32.dll already exists!
    echo Delete it first if you want to re-download.
    goto :end
)

echo Downloading Mesa3D from GitHub...
echo.

REM Download the 7z archive from mmozeiko/build-mesa
set MESA_URL=https://github.com/mmozeiko/build-mesa/releases/download/25.3.1/mesa-llvmpipe-x64-25.3.1.7z
set MESA_FILE=mesa-temp.7z

powershell -Command "& {[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; Invoke-WebRequest -Uri '%MESA_URL%' -OutFile '%MESA_FILE%'}" 2>nul

if not exist %MESA_FILE% (
    echo Download failed!
    echo.
    echo Please download manually:
    echo   1. Go to: https://github.com/mmozeiko/build-mesa/releases
    echo   2. Download mesa-llvmpipe-x64-*.7z
    echo   3. Extract opengl32.dll to this folder
    goto :end
)

echo Download complete! Extracting opengl32.dll...

REM Try PowerShell 5+ Expand-Archive (only works with zip, not 7z)
REM We need 7-Zip or tar for .7z files

REM Check for 7-Zip
where 7z >nul 2>&1
if %errorlevel% equ 0 (
    7z e %MESA_FILE% opengl32.dll -y >nul
    goto :check_result
)

REM Check for 7za
where 7za >nul 2>&1
if %errorlevel% equ 0 (
    7za e %MESA_FILE% opengl32.dll -y >nul
    goto :check_result
)

REM Check common 7-Zip installation paths
if exist "C:\Program Files\7-Zip\7z.exe" (
    "C:\Program Files\7-Zip\7z.exe" e %MESA_FILE% opengl32.dll -y >nul
    goto :check_result
)

if exist "C:\Program Files (x86)\7-Zip\7z.exe" (
    "C:\Program Files (x86)\7-Zip\7z.exe" e %MESA_FILE% opengl32.dll -y >nul
    goto :check_result
)

REM No 7-Zip found
echo.
echo ERROR: 7-Zip is required to extract the Mesa3D archive.
echo.
echo Please either:
echo   1. Install 7-Zip from https://www.7-zip.org/
echo   2. Or manually extract opengl32.dll from %MESA_FILE%
echo.
echo The archive has been downloaded to: %MESA_FILE%
goto :end

:check_result
del %MESA_FILE% 2>nul

if exist opengl32.dll (
    echo.
    echo ============================================
    echo  SUCCESS! Mesa3D installed successfully!
    echo ============================================
    echo.
    echo opengl32.dll is now in place.
    echo Run chess.exe to play with software rendering.
    echo.
) else (
    echo.
    echo Extraction failed. Please extract manually:
    echo   1. Open %MESA_FILE% with 7-Zip
    echo   2. Copy opengl32.dll to this folder
)

:end
echo.
pause
