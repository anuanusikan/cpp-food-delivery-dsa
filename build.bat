@echo off
set PATH=C:\msys64\mingw64\bin;%PATH%
echo Compiling food delivery system...
g++ -std=c++17 -O2 -D_WIN32_WINNT=0x0A00 -static -o food_server.exe main.cpp ds\*.cpp Services\*.cpp -lws2_32
if %errorlevel% neq 0 (
    echo Compilation failed!
    exit /b %errorlevel%
)
echo Compilation successful. Run food_server.exe to start the server!
