# =============================================================================
# Mesa3D Software Renderer Download Script
# =============================================================================
# Use this script if you get OpenGL errors when running chess.exe
# It downloads a software OpenGL renderer that works without GPU drivers.
#
# Usage: Right-click -> Run with PowerShell
#    or: powershell -ExecutionPolicy Bypass -File download-mesa.ps1
# =============================================================================

$ErrorActionPreference = "Stop"
$ProjectDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $ProjectDir

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Mesa3D Software Renderer Download" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$MesaVersion = "25.3.1"
$MesaUrl = "https://github.com/mmozeiko/build-mesa/releases/download/$MesaVersion/mesa-llvmpipe-x64-$MesaVersion.7z"
$MesaArchive = "$env:TEMP\mesa.7z"
$OpenGLDll = "$ProjectDir\opengl32.dll"

if (Test-Path $OpenGLDll) {
    Write-Host "[OK] opengl32.dll already exists" -ForegroundColor Green
    Write-Host "     Delete it first if you want to re-download." -ForegroundColor Gray
    exit 0
}

# Check for 7-Zip
$7zPath = $null
$7zLocations = @(
    "C:\Program Files\7-Zip\7z.exe",
    "C:\Program Files (x86)\7-Zip\7z.exe",
    "$env:USERPROFILE\MinGW\bin\7z.exe"
)
foreach ($loc in $7zLocations) {
    if (Test-Path $loc) { $7zPath = $loc; break }
}

if (-not $7zPath) {
    # Try to use PowerShell's Expand-Archive with tar (Windows 10+)
    Write-Host "[!] 7-Zip not found. Attempting alternative extraction..." -ForegroundColor Yellow
}

Write-Host "[*] Downloading Mesa3D ($MesaVersion)..." -ForegroundColor Yellow
Invoke-WebRequest -Uri $MesaUrl -OutFile $MesaArchive -UseBasicParsing

if ($7zPath) {
    Write-Host "[*] Extracting with 7-Zip..." -ForegroundColor Yellow
    & $7zPath e $MesaArchive -o"$ProjectDir" opengl32.dll -y | Out-Null
} else {
    Write-Host "[!] Please install 7-Zip from https://www.7-zip.org/" -ForegroundColor Red
    Write-Host "    Then extract opengl32.dll from: $MesaArchive" -ForegroundColor Yellow
    Write-Host "    Place it in: $ProjectDir" -ForegroundColor Yellow
    exit 1
}

Remove-Item $MesaArchive -Force -ErrorAction SilentlyContinue

if (Test-Path $OpenGLDll) {
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Green
    Write-Host "  Mesa3D installed successfully!" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "The game will now use software rendering." -ForegroundColor Cyan
    Write-Host "Run: .\chess.exe" -ForegroundColor Cyan
} else {
    Write-Host "[ERROR] Extraction failed" -ForegroundColor Red
    exit 1
}
