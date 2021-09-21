@echo off
set PGCC=\Storage Card\アプリ\Tools\PocketGCC 1.50\pgcc
set SRC=\Storage Card\アプリ\Test

echo *** win.cpp をコンパイル中です...
if exist "%SRC%\win.s" del "%SRC%\win.s"
"%PGCC%\cc1plus" "%SRC%\win.cpp" -o "%SRC%\win.s" -I "%PGCC%\include" -I "%SRC%" -include "%PGCC%\fixincl.h" -fms-extensions
if not exist "win.s" goto errors

echo *** win.s をアセンブル中です...
if exist "%SRC%\win.o" del "%SRC%\win.o"
"%PGCC%\as" "%SRC%\win.s" -o "%SRC%\win.o"
if not exist "win.o" goto errors

if exist "resource.rc" goto withres

echo *** win.o を AppMain.exe にリンク中です...
"%PGCC%\ld" "%SRC%\win.o" -o "%SRC%\AppMain.exe" -L "%PGCC%\lib" -l cpplib -l corelibc -l coredll -l aygshell -l runtime -l portlib -e WinMain
if not exist "AppMain.exe" goto errors
goto success

:withres
echo *** リソーススクリプト resource.rc を前処理中です...
if exist "resource.rc.p" del "resource.rc.p"
"%pgcc%\cpp0" "%src%\resource.rc" -o "%SRC%\resource.rc.p" -I "%pgcc%\include" -I "%SRC%" -DRC_INVOKED -include "%pgcc%\fixincl.h"
if not exist "resource.rc.p" goto errors

echo *** 前処理済みリソース resource.rc.p をアセンブル中です...
if exist "resource.rc.o" del "resource.rc.o"
"%pgcc%\windres" "%SRC%\resource.rc.p" -o "%SRC%\resource.rc.o" --include-dir "%src%"
if not exist "resource.rc.o" goto errors

echo *** win.o と resource.rc.o を AppMain.exe にリンク中です...
"%PGCC%\ld" "%SRC%\win.o" "%SRC%\resource.rc.o" -o "%SRC%\AppMain.exe" -L "%PGCC%\lib" -l cpplib -l commctrl -l corelibc -l coredll -l aygshell -l runtime -l portlib -e WinMain
if not exist "AppMain.exe" goto errors

:success
echo ビルドが正常に完了しました。
exit

:errors
echo 中断: ビルド中にエラーが発生しました。