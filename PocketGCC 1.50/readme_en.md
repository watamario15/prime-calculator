# Prime Factorization Software's Source Code for PocketGCC 1.50
この文書には[日本語版](readme.md)もあります。

This source code is for **Windows CE (StrongARM)**.

## Building Requirements
Make your Windows CE device usable the PocketGCC and a command shell, and use the attached batch file. You may need to edit the batch file depending on your environment, especially paths.

I use commctrl.lib copied from the Standard SDK for eMbedded Visual C++ 4.0 to compile. This library is needed to display a command bar, and you must **get it in the same way, use the dynamic linking, or delete the command bar**. (There might be other PocketGCCs that don't require these process.)

This compiler **doesn't support source files includes non-ASCII characters**. If you want to build software including non-ASCII characters, consider putting them in the String Table of a resource script, or using another compiler such as eMbedded Visual C++ 4.0 or CeGCC.

## Notes
The source code is "**win.cpp**", the resource script is "**resource.rc**", the icon is "**app.ico**", and the batch file for building is "**WINBUILD.BAT**". The executable binary file will be generated as "**AppMain.exe**".