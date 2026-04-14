@echo off
set PATH=%~dp0w64devkit\bin;%PATH%
echo Compiling Lux (Modular)...
g++ main.cpp Gameplay/TargetSystem.cpp Renderer/Renderer.cpp Core/Globals.cpp Modes/*.cpp -o Lux.exe -lgdi32 -luser32 -mwindows
if %errorlevel% equ 0 (
    echo Compilation successful!
    start .\Lux.exe
) else (
    echo Compilation failed.
)
pause
