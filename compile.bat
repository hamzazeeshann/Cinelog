@echo off
echo =====================================
echo  Compiling Cinelog Backend
echo =====================================
echo.

cd backend
g++ -std=c++17 -o server.exe src/main.cpp -lws2_32

if %ERRORLEVEL% EQU 0 (
    echo.
    echo =====================================
    echo  Compilation Successful!
    echo =====================================
    echo.
    echo Run 'start_server.bat' to start the server
) else (
    echo.
    echo =====================================
    echo  Compilation Failed!
    echo =====================================
)

pause
