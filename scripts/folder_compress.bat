@echo off
setlocal enabledelayedexpansion

:: ============================================================
:: Batch-скрипт для пакетной конвертации glTF → KTX2
:: Вызов: texcompr -m <file.gltf>
:: ============================================================

if "%~1"=="" (
    echo Usage: %~nx0 ^<folder^>
    pause
    exit /b 1
)

if not exist "%~1" (
    echo [ERROR] Folder not found: %~1
    pause
    exit /b 1
)

where texcompr >nul 2>&1
if errorlevel 1 (
    echo [ERROR] texcompr.exe not found! Place it in PATH or current dir.
    pause
    exit /b 1
)

set "INPUT_DIR=%~1"
set "COUNT=0"
set "FAILED=0"

echo ============================================================
echo GLTF to KTX2 Batch Converter
echo ============================================================
echo Input: %INPUT_DIR%
echo Command: texcompr -m ^<file.gltf^>
echo ============================================================
echo.

for /r "%INPUT_DIR%" %%f in (*.gltf) do (
    set "FNAME=%%~nxf"
    set "SKIP=0"

    

    :: ВАЖНО: ) и else ( ДОЛЖНЫ быть на одной строке!
    if !SKIP! equ 1 (
        echo [SKIP] !FNAME!
    ) else (
        set /a COUNT+=1
        echo.
        echo [!COUNT!] Processing: %%f
        echo --------------------------------------------------
        
        :: call гарантирует корректный возврат в цикл после выполнения
        texcompr -m "%%f" -g -o -i
        if errorlevel 1 (
            echo [ERROR] Failed to convert: %%f
            set /a FAILED+=1
        )
    )
)

echo.
echo ============================================================
echo Complete! Processed: %COUNT%, Failed: %FAILED%
echo Success: %COUNT% - %FAILED%
echo ============================================================
pause
if %FAILED% gtr 0 exit /b 1
exit /b 0