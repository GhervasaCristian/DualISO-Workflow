@echo off
setlocal enabledelayedexpansion

:: Set the directory where the files are located (modify this line to your folder path)
set "folder_path=L:\Nunta Andrei Andra Tucaliuc\Dual ISO CR2\DNG+NDISO"

:: Change to the specified directory
cd /d "%folder_path%"

:: Loop through all .dng files
for %%f in (*.dng) do (
    set "filename=%%f"
    :: Check if "DxO_DeepPRIME 3" exists in the filename
    echo !filename! | findstr /i "DxO_DeepPRIME 3" >nul
    if !errorlevel! equ 0 (
        echo Deleting !filename!
        del "%%f"
    )
)

echo Done!
pause
