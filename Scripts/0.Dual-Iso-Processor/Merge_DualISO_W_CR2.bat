@echo off
setlocal enabledelayedexpansion

REM Create target folders if they don’t exist
mkdir "DNG+NDISO" >nul 2>&1
mkdir "ORIGINAL+DISO" >nul 2>&1


REM Clean up any existing temp files
if exist files.tmp del files.tmp

REM STEP 1: Process all CR2 files
for %%F in (*.CR2) do (
    set "base=%%~nF"
    if exist "!base!.DNG" (
        REM Case 1: Both CR2 and DNG exist → move only DNG
        move "!base!.DNG" "DNG+NDISO\" >nul
    ) else (
        REM Case 2: Only CR2 exists → move it
        move "%%F" "DNG+NDISO\" >nul
    )
)

REM STEP 2: Copy ALL CR2 files (from original location and DNG+NDISO) to ORIGINAL+DISO
REM Including the ones moved to DNG+NDISO (since they are no longer in current dir)
for %%F in ("*.CR2") do (
    move "%%F" "ORIGINAL+DISO\" >nul
)

for %%F in ("DNG+NDISO\*.CR2") do (
    copy "%%F" "ORIGINAL+DISO\" >nul
)

REM STEP 3: Summary check
for /f %%A in ('dir /b /a-d "DNG+NDISO" ^| find /v /c ""') do set dngCount=%%A
for /f %%B in ('dir /b /a-d "ORIGINAL+DISO" ^| find /v /c ""') do set cr2Count=%%B

echo.
echo Processing complete.
echo Files in DNG+NDISO: %dngCount%
echo Files in ORIGINAL+DISO (CR2 only): %cr2Count%

pause
endlocal
