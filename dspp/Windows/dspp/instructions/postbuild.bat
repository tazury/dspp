@echo off
setlocal enabledelayedexpansion

:: Set the source and destination directories
set SOURCE_DIR=%1
set BUILD_DIR=%2
set TPTY_DIR=%3

:: Create the target directories if they don't exist
if not exist "%BUILD_DIR%\include" mkdir "%BUILD_DIR%\include"
if not exist "%BUILD_DIR%\libs" mkdir "%BUILD_DIR%\libs"

:: Copy .h files to the include folder
echo Copying .h files to %BUILD_DIR%\include...
for %%F in (%SOURCE_DIR%\*.h) do (
    copy "%%F" "%BUILD_DIR%\include\"
)

:: Move 3rd Party files to the include folder.
echo Copying 3rd Party includes	
for /d /r "%TPTY_DIR%" %%F in (include) do (
    echo Copying %%F to %DEST_DIR%...
    set "PARENT=%%~dpF"
    for %%A in ("!PARENT:~0,-1!") do set "PARENT_NAME=%%~nxA"

    xcopy /e /i /y "%%F" "%BUILD_DIR%\include\!PARENT_NAME!\include\"
)

:: Move .lib file to the libs folder
echo Moving .lib files to %BUILD_DIR%\libs...
for %%F in (%BUILD_DIR%\*) do (
    move "%%F" "%BUILD_DIR%\libs\"
)

echo Post-build process complete.

endlocal
