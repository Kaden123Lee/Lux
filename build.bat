@echo off
echo Compiling Lux (Modular)...
g++ main.cpp TargetSystem.cpp Renderer.cpp Globals.cpp -o Lux.exe -lgdi32 -luser32 -mwindows
if %errorlevel% equ 0 (
    echo Compilation successful!
    start .\Lux.exe
) else (
    echo Compilation failed.
)
pause
