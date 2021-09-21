#!/usr/bin/env bash

SRC=main.cpp
RES=resource.rc

if [ -e "${RES}" ]; then
    # withres
    echo "*** C++ ソース (${SRC}) をコンパイル+アセンブル中..."
    if [ -e "obj32.o" ]; then
        rm -f "obj32.o"
    fi
    if [ -e "obj64.o" ]; then
        rm -f "obj64.o"
    fi
    i686-w64-mingw32-g++ ${SRC} -Wall -O2 -std=gnu++17 -DUNICODE -D_UNICODE -c -o obj32.o
    x86_64-w64-mingw32-g++ ${SRC} -Wall -O2 -std=gnu++17 -DUNICODE -D_UNICODE -c -o obj64.o
    if ! [ -e "obj32.o" -a -e "obj64.o" ]; then
        echo "ビルド失敗"; exit
    fi

    echo "*** リソース (${RES}) を前処理+アセンブル中..."
    if [ -e "res32.o" ]; then
        rm -f "res32.o"
    fi
    if [ -e "res64.o" ]; then
        rm -f "res64.o"
    fi
    i686-w64-mingw32-windres ${RES} res32.o
    x86_64-w64-mingw32-windres ${RES} res64.o
    if ! [ -e "res32.o" -a -e "res64.o" ]; then
        echo "ビルド失敗"; exit
    fi

    echo "*** obj32.o と res32.o をリンク中..."
    i686-w64-mingw32-g++ obj32.o res32.o -Wall -mwindows -static -s -o PF_IA-32.exe
    x86_64-w64-mingw32-g++ obj64.o res64.o -Wall -mwindows -static -s -o PF_AMD64.exe
    if ! [ -e "PF_IA-32.exe" -a -e "PF_AMD64.exe" ]; then
        echo "ビルド失敗"; exit
    fi
else
    echo "*** C++ ソース (${SRC}) をビルド中..."
    i686-w64-mingw32-g++ "${SRC}" -Wall -O2 -std=gnu++17 -mwindows -DUNICODE -D_UNICODE -static -s -o PF_IA-32.exe
    x86_64-w64-mingw32-g++ "${SRC}" -Wall -O2 -std=gnu++17 -mwindows -DUNICODE -D_UNICODE -static -s -o PF_AMD64.exe
    if ! [ -e "PF_IA-32.exe" -a -e "PF_AMD64.exe" ]; then
        echo "ビルド失敗"; exit
    fi
fi

echo "ビルド成功"