@echo off
setlocal enabledelayedexpansion

:: 1. Define folder names and target paths
set "GlobalBinFolder=%USERPROFILE%\axlflash"
set "BinaryToInstall=axlflash.exe"

:: 2. Create the global folder if it doesn't exist
if not exist "%GlobalBinFolder%" (
    mkdir "%GlobalBinFolder%"
    echo Created global folder at: %GlobalBinFolder%
)

:: 3. Copy the compiled binary
if exist "%BinaryToInstall%" (
    copy /Y "%BinaryToInstall%" "%GlobalBinFolder%\" >nul
    echo Successfully copied %BinaryToInstall% to global folder.
) else (
    echo WARNING: Could not find "%BinaryToInstall%" in the current directory.
)

:: 5. Permanently add the folder to the User PATH if it's not already there
:: Retrieve current User PATH from the registry
for /f "tokens=2*" %%A in ('reg query "HKCU\Environment" /v Path 2^>nul') do set "OldPath=%%B"

:: Check if the folder is already in the PATH
echo !OldPath! | findstr /I /C:"%GlobalBinFolder%" >nul
if %errorlevel% neq 0 (
    :: If there's an existing path, append to it; otherwise create it new
    if defined OldPath (
        setx PATH "%OldPath%;%GlobalBinFolder%" >nul
    ) else (
        setx PATH "%GlobalBinFolder%" >nul
    )
    echo Added %GlobalBinFolder% to your User PATH.
    echo Please RESTART your command prompt for changes to take effect.
) else (
    echo Folder is already in your PATH.
)

pause
