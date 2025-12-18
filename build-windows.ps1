# =============================================================================
# C-hess Windows Build Script
# =============================================================================
# This script automatically sets up MinGW, downloads dependencies, and builds
# the chess game on Windows using PowerShell (no MSYS2 required).
#
# Usage: Right-click -> Run with PowerShell
#    or: powershell -ExecutionPolicy Bypass -File build-windows.ps1
# =============================================================================

$ErrorActionPreference = "Stop"
$ProjectDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $ProjectDir

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  C-hess Windows Build Script" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# -----------------------------------------------------------------------------
# Step 1: Check/Install MinGW
# -----------------------------------------------------------------------------
$MinGWPath = "$env:USERPROFILE\MinGW"
$GccPath = "$MinGWPath\bin\gcc.exe"

if (Test-Path $GccPath) {
    Write-Host "[OK] MinGW already installed at $MinGWPath" -ForegroundColor Green
} else {
    Write-Host "[*] Downloading MinGW portable (~60MB)..." -ForegroundColor Yellow
    $MinGWUrl = "https://nuwen.net/files/mingw/mingw-19.0.exe"
    $MinGWInstaller = "$env:TEMP\mingw-portable.exe"
    
    Invoke-WebRequest -Uri $MinGWUrl -OutFile $MinGWInstaller -UseBasicParsing
    Write-Host "[*] Extracting MinGW to $MinGWPath..." -ForegroundColor Yellow
    Start-Process -FilePath $MinGWInstaller -ArgumentList "-y", "-o$env:USERPROFILE" -Wait
    
    if (Test-Path $GccPath) {
        Write-Host "[OK] MinGW installed successfully" -ForegroundColor Green
    } else {
        Write-Host "[ERROR] MinGW installation failed" -ForegroundColor Red
        exit 1
    }
}

# Add MinGW to PATH for this session
$env:PATH = "$MinGWPath\bin;$env:PATH"

# Verify gcc works
$GccVersion = & gcc --version 2>&1 | Select-Object -First 1
Write-Host "[OK] Using $GccVersion" -ForegroundColor Green
Write-Host ""

# -----------------------------------------------------------------------------
# Step 2: Clone raylib if not present
# -----------------------------------------------------------------------------
$RaylibDir = "$ProjectDir\raylib"
$RaylibLib = "$RaylibDir\src\libraylib.a"

if (Test-Path $RaylibLib) {
    Write-Host "[OK] raylib already built" -ForegroundColor Green
} else {
    if (-not (Test-Path "$RaylibDir\src")) {
        Write-Host "[*] Cloning raylib..." -ForegroundColor Yellow
        if (Test-Path $RaylibDir) { Remove-Item $RaylibDir -Recurse -Force }
        git clone --depth 1 https://github.com/raysan5/raylib.git $RaylibDir
    }
    
    Write-Host "[*] Building raylib (this may take a minute)..." -ForegroundColor Yellow
    Push-Location "$RaylibDir\src"
    & make PLATFORM=PLATFORM_DESKTOP
    Pop-Location
    
    if (Test-Path $RaylibLib) {
        Write-Host "[OK] raylib built successfully" -ForegroundColor Green
    } else {
        Write-Host "[ERROR] raylib build failed" -ForegroundColor Red
        exit 1
    }
}
Write-Host ""

# -----------------------------------------------------------------------------
# Step 3: Clone and build libjuice if not present
# -----------------------------------------------------------------------------
$LibjuiceDir = "$ProjectDir\libjuice"
$LibjuiceLib = "$LibjuiceDir\libjuice.a"

if (Test-Path $LibjuiceLib) {
    Write-Host "[OK] libjuice already built" -ForegroundColor Green
} else {
    if (-not (Test-Path "$LibjuiceDir\src\juice.c")) {
        Write-Host "[*] Cloning libjuice..." -ForegroundColor Yellow
        if (Test-Path $LibjuiceDir) { Remove-Item $LibjuiceDir -Recurse -Force }
        git clone --depth 1 https://github.com/paullouisageneau/libjuice.git $LibjuiceDir
    }
    
    Write-Host "[*] Building libjuice..." -ForegroundColor Yellow
    $JuiceSources = @(
        "addr", "agent", "base64", "conn", "conn_mux", "conn_poll", "conn_thread",
        "const_time", "crc32", "hash", "hmac", "ice", "juice", "log", "random",
        "server", "stun", "tcp", "timestamp", "turn", "udp"
    )
    
    foreach ($src in $JuiceSources) {
        $srcFile = "$LibjuiceDir\src\$src.c"
        $objFile = "$LibjuiceDir\src\$src.o"
        Write-Host "  Compiling $src.c..." -ForegroundColor Gray
        & gcc -O2 -fPIC -fvisibility=hidden -DJUICE_EXPORTS -DUSE_NETTLE=0 `
              -I"$LibjuiceDir\include\juice" -c $srcFile -o $objFile
    }
    
    Write-Host "  Creating libjuice.a..." -ForegroundColor Gray
    & ar rcs $LibjuiceLib "$LibjuiceDir\src\*.o"
    
    if (Test-Path $LibjuiceLib) {
        Write-Host "[OK] libjuice built successfully" -ForegroundColor Green
    } else {
        Write-Host "[ERROR] libjuice build failed" -ForegroundColor Red
        exit 1
    }
}
Write-Host ""

# -----------------------------------------------------------------------------
# Step 4: Build the chess game
# -----------------------------------------------------------------------------
Write-Host "[*] Building chess.exe..." -ForegroundColor Yellow

$Sources = @("main", "board", "moves", "check", "ui", "menu", "history", "constants", "clock", "network", "multiplayer")
$Objects = @()

foreach ($src in $Sources) {
    $srcFile = "$ProjectDir\$src.c"
    $objFile = "$ProjectDir\$src.o"
    $Objects += $objFile
    
    # Check if source is newer than object
    $needsCompile = $true
    if (Test-Path $objFile) {
        $srcTime = (Get-Item $srcFile).LastWriteTime
        $objTime = (Get-Item $objFile).LastWriteTime
        if ($objTime -gt $srcTime) { $needsCompile = $false }
    }
    
    if ($needsCompile) {
        Write-Host "  Compiling $src.c..." -ForegroundColor Gray
        & gcc -Wall -Wextra -O2 -I"$RaylibDir\src" -I"$LibjuiceDir\include" -DJUICE_STATIC `
              -c $srcFile -o $objFile
    }
}

Write-Host "  Linking chess.exe..." -ForegroundColor Gray
$ObjectsStr = $Objects -join " "
$LinkCmd = "gcc -o chess.exe $ObjectsStr `"$RaylibLib`" `"$LibjuiceLib`" -lopengl32 -lgdi32 -lwinmm -lws2_32 -lbcrypt -static -lpthread"
Invoke-Expression $LinkCmd

if (Test-Path "$ProjectDir\chess.exe") {
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Green
    Write-Host "  BUILD SUCCESSFUL!" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "Run the game with: .\chess.exe" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "If you get OpenGL errors, run: .\download-mesa.ps1" -ForegroundColor Yellow
} else {
    Write-Host "[ERROR] Linking failed" -ForegroundColor Red
    exit 1
}
