@echo off
:: Ensure the script runs in the directory where this file is located
cd /d "%~dp0"

echo Processing folders in: %CD%
echo ------------------------------

:: 1. Check if "IN" exists and delete it
if exist "IN" (
    echo Found "IN" folder. Deleting...
    rd /s /q "IN"
    echo "IN" folder deleted.
) else (
    echo "IN" folder does not exist. Skipping delete.
)

echo.

:: 2. Check if "xIN" exists, then copy it as "IN"
if exist "xIN" (
    echo Found "xIN". Copying to new "IN" folder...
    :: /E = Copy subdirectories, including empty ones
    :: /I = Assume destination is a folder
    :: /Y = Suppress prompt to overwrite (just in case)
    xcopy "xIN" "IN" /E /I /Y
    echo Done! "xIN" has been copied to "IN".
) else (
    echo ERROR: Folder "xIN" was not found in this directory!
)

:: 1. Check if "OUT" exists and delete it
if exist "OUT" (
    echo Found "OUT" folder. Deleting...
    rd /s /q "OUT"
    echo "OUT" folder deleted.
) else (
    echo "OUT" folder does not exist. Skipping delete.
)

echo.

:: 2. Check if "xOUT" exists, then copy it as "OUT"
if exist "xOUT" (
    echo Found "xOUT". Copying to new "OUT" folder...
    :: /E = Copy subdirectories, including empty ones
    :: /I = Assume destination is a folder
    :: /Y = Suppress prompt to overwrite (just in case)
    xcopy "xOUT" "OUT" /E /I /Y
    echo Done! "xOUT" has been copied to "OUT".
) else (
    echo ERROR: Folder "xOUT" was not found in this directory!
)
