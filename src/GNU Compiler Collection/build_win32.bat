@echo off

set PATH=C:\MinGW\bin
cd /d %~dp0

if exist "resource.rc" goto withres

echo *** C++ソース(1.cpp)をビルド中...
g++ -Wall -o PF_IA-32.exe -static-libstdc++ -static-libgcc -mwindows -DUNICODE 1.cpp
if not exist "PF_IA-32.exe" goto errors
goto success

:withres
echo *** C++ソース(1.cpp)をコンパイル+アセンブル中...
if exist "obj.o" del "obj.o"
g++ -Wall -c 1.cpp -o obj.o -DUNICODE
if not exist "obj.o" goto errors

echo *** リソース(resource.rc)を前処理+アセンブル中...
if exist "resource.rc.o" del "resource.rc.o"
windres resource.rc resource.rc.o -DUNICODE
if not exist "resource.rc.o" goto errors

echo *** obj.oとresource.rc.oをリンク中...
g++ -Wall obj.o resource.rc.o -o PF_IA-32.exe -static-libstdc++ -static-libgcc -mwindows -DUNICODE
if not exist "PF_IA-32.exe" goto errors
goto success


:success
echo ビルド成功
PF_IA-32.exe
exit /B

:errors
echo ビルド失敗
pause
exit /B