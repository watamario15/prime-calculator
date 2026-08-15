@echo off
set PGCC=\Storage Card\pgcc
set SRC=\Storage Card\src

echo *** Compiling C++ sources...
"%PGCC%\cc1plus" "%SRC%\main.cpp" -o "%SRC%\main.s" -I "%PGCC%\include" -I "%SRC%" -include "%PGCC%\fixincl.h" -fms-extensions
"%PGCC%\cc1plus" "%SRC%\msg.cpp" -o "%SRC%\msg.s" -I "%PGCC%\include" -I "%SRC%" -include "%PGCC%\fixincl.h" -fms-extensions
"%PGCC%\cc1plus" "%SRC%\runner.cpp" -o "%SRC%\runner.s" -I "%PGCC%\include" -I "%SRC%" -include "%PGCC%\fixincl.h" -fms-extensions
"%PGCC%\cc1plus" "%SRC%\ui.cpp" -o "%SRC%\ui.s" -I "%PGCC%\include" -I "%SRC%" -include "%PGCC%\fixincl.h" -fms-extensions
"%PGCC%\cc1plus" "%SRC%\util.cpp" -o "%SRC%\util.s" -I "%PGCC%\include" -I "%SRC%" -include "%PGCC%\fixincl.h" -fms-extensions
"%PGCC%\cc1plus" "%SRC%\wproc.cpp" -o "%SRC%\wproc.s" -I "%PGCC%\include" -I "%SRC%" -include "%PGCC%\fixincl.h" -fms-extensions
echo *** Assembling...
"%PGCC%\as" "%SRC%\main.s" -o "%SRC%\main.o"
"%PGCC%\as" "%SRC%\msg.s" -o "%SRC%\msg.o"
"%PGCC%\as" "%SRC%\runner.s" -o "%SRC%\runner.o"
"%PGCC%\as" "%SRC%\ui.s" -o "%SRC%\ui.o"
"%PGCC%\as" "%SRC%\util.s" -o "%SRC%\util.o"
"%PGCC%\as" "%SRC%\wproc.s" -o "%SRC%\wproc.o"
echo *** Preprocessing resource-pgcc.rc...
"%PGCC%\cpp0" "%SRC%\resource-pgcc.rc" -o "%SRC%\resource.p" -I "%PGCC%\include" -I "%SRC%" -DRC_INVOKED -include "%PGCC%\fixincl.h"
echo *** Compiling resource.p...
"%PGCC%\windres" "%SRC%\resource.p" -o "%SRC%\resource.o" --include-dir "%SRC%"
echo *** Linking...
"%PGCC%\ld"  "%SRC%\main.o" "%SRC%\msg.o" "%SRC%\runner.o" "%SRC%\ui.o" "%SRC%\util.o" "%SRC%\wproc.o" "%SRC%\resource.o" -o "%SRC%\AppMain.exe" -L "%PGCC%\lib" -lcpplib -lcorelibc -lcoredll -lruntime -lportlib -lcommctrl -eWinMain
