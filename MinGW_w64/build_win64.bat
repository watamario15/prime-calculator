@echo off
chcp 65001 >nul 2>&1

set PATH=C:\mingw64\bin
set SRC=main.cpp
set RES=resource.rc
cd /d %~dp0

if exist "%RES%" goto withres

echo *** C++ ソース (%SRC%) をビルド中...
g++ "%SRC%" -Wall -O2 -std=gnu++17 -mwindows -DUNICODE -D_UNICODE -static -s -o PF_AMD64.exe
if not exist "PF_AMD64.exe" goto error
goto success

:withres
echo *** C++ ソース (%SRC%) をコンパイル+アセンブル中...
if exist "obj.o" del "obj.o"
g++ "%SRC%" -Wall -O2 -std=gnu++17 -DUNICODE -D_UNICODE -c -o obj.o
if not exist "obj.o" goto error

echo *** リソース (%RES%) を前処理+アセンブル中...
if exist "res.o" del "res.o"
windres "%RES%" res.o
if not exist "res.o" goto error

echo *** obj.o と res.o をリンク中...
g++ obj.o res.o -Wall -mwindows -static -s -o PF_AMD64.exe
if not exist "PF_AMD64.exe" goto error
goto success


:success
echo ビルド成功
PF_AMD64.exe
exit /B

:error
echo ビルド失敗
pause
exit /B