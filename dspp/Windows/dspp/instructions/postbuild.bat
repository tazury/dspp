@echo off
setlocal enabledelayedexpansion

:: Set the source and destination directories
set SOURCE_DIR=%1
set BUILD_DIR=%2
set TPTY_DIR=%3

if not exist "%BUILD_DIR%\dspp" mkdir "%BUILD_DIR%\dspp"

:: Create the target directories if they don't exist
if not exist "%BUILD_DIR%\dspp\include" mkdir "\%BUILD_DIR%\dspp\include"
if not exist "%BUILD_DIR%\dspp\libs" mkdir "%BUILD_DIR%\dspp\libs"

:: Copy .h files to the include folder
set "SOURCE_DIRS_SPACED=%SOURCE_DIR:,= %"

REM Iterate over each directory
set "REMAINING=%SOURCE_DIR%"

:parse_loop
for /f "tokens=1* delims=," %%A in ("!REMAINING!") do (
    set "CURRENT_DIR=%%A"
    set "REMAINING=%%B"

    echo Processing !CURRENT_DIR!...
    for %%F in ("!CURRENT_DIR!\*.h") do (
        echo Copying %%F
        copy "%%F" "%BUILD_DIR%\dspp\include\"
    )

    if defined REMAINING goto parse_loop
)
:: Move 3rd Party files to the include folder.
echo Copying 3rd Party includes	
for /d /r "%TPTY_DIR%" %%F in (include) do (
    echo Copying %%F
    set "PARENT=%%~dpF"
    for %%A in ("!PARENT:~0,-1!") do set "PARENT_NAME=%%~nxA"

    xcopy /e /i /y "%%F" "%BUILD_DIR%\dspp\include\!PARENT_NAME!\include\"
)

:: Move .lib file to the libs folder
echo Moving .lib files to %BUILD_DIR%\libs...
for %%F in (%BUILD_DIR%\*) do (
    move "%%F" "%BUILD_DIR%\dspp\libs\"
)

echo Post-build process complete.

endlocal
