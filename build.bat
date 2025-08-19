@echo off
echo === TCP Group Chat Application Build Script ===
echo.

REM Check if we're in the right directory
if not exist "common\protocol.h" (
    echo Error: Please run this script from the project root directory
    pause
    exit /b 1
)

REM Create target directory
if not exist "target" mkdir target

echo Building TCP Group Chat Application...
echo.

REM Try to find available compilers
set COMPILER=
set COMPILER_FLAGS=

REM Check for GCC (MinGW)
where gcc >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    set COMPILER=gcc
    set COMPILER_FLAGS=-Wall -Wextra -std=c99 -g
    echo Found GCC compiler
    goto :compile
)

REM Check for Clang
where clang >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    set COMPILER=clang
    set COMPILER_FLAGS=-Wall -Wextra -std=c99 -g
    echo Found Clang compiler
    goto :compile
)

REM Check for MSVC (Visual Studio)
where cl >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    set COMPILER=cl
    set COMPILER_FLAGS=/W3 /std:c11
    echo Found MSVC compiler
    goto :compile
)

echo Error: No C compiler found!
echo Please install one of the following:
echo - MinGW-w64 (GCC for Windows)
echo - Clang for Windows
echo - Visual Studio Build Tools
echo.
echo You can also use Docker to build the project:
echo docker build -t tcp-chat .
echo.
pause
exit /b 1

:compile
echo.
echo Compiling with %COMPILER%...
echo.

REM Compile common library
echo Compiling common library...
%COMPILER% %COMPILER_FLAGS% -c common\list.c -o target\list.o
if %ERRORLEVEL% NEQ 0 (
    echo Error compiling common library
    pause
    exit /b 1
)

REM Compile server
echo Compiling server...
%COMPILER% %COMPILER_FLAGS% -c server\auth.c -o target\auth.o
%COMPILER% %COMPILER_FLAGS% -c server\network.c -o target\network.o
%COMPILER% %COMPILER_FLAGS% -c server\server.c -o target\server.o
if %ERRORLEVEL% NEQ 0 (
    echo Error compiling server
    pause
    exit /b 1
)

REM Compile client
echo Compiling client...
%COMPILER% %COMPILER_FLAGS% -c client\auth.c -o target\client_auth.o
%COMPILER% %COMPILER_FLAGS% -c client\network.c -o target\client_network.o
%COMPILER% %COMPILER_FLAGS% -c client\client.c -o target\client_main.o
if %ERRORLEVEL% NEQ 0 (
    echo Error compiling client
    pause
    exit /b 1
)

REM Link server
echo Linking server...
%COMPILER% target\list.o target\auth.o target\network.o target\server.o -o target\server.exe
if %ERRORLEVEL% NEQ 0 (
    echo Error linking server
    pause
    exit /b 1
)

REM Link client
echo Linking client...
%COMPILER% target\list.o target\client_auth.o target\client_network.o target\client_main.o -o target\client.exe
if %ERRORLEVEL% NEQ 0 (
    echo Error linking client
    pause
    exit /b 1
)

echo.
echo === Build completed successfully! ===
echo.
echo Executables created:
echo   - target\server.exe
echo   - target\client.exe
echo.
echo To run the application:
echo 1. Start server: target\server.exe 0.0.0.0 8080
echo 2. Start client: target\client.exe 127.0.0.1 8080
echo.
echo Or use Docker (recommended):
echo 1. Start server: docker compose run --rm --name server server
echo 2. Start client: docker compose run --rm --name client1 client1
echo.
pause
