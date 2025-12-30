@echo off
setlocal enabledelayedexpansion

:: This script replaces the prefix "DEN__" with a single underscore "_"
echo Starting rename process...
echo.

for %%F in (DEN__*) do (
    set "oldname=%%~nxF"
    
    :: Remove the first 5 characters (DEN__) and add an underscore at the start
    set "newname=_!oldname:~5!"
    
    echo Renaming: "%%F" --^> "!newname!"
    ren "%%F" "!newname!"
)

echo.
echo Task Complete!
pause