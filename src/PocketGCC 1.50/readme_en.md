# Prime Factorization Software Ver. 2 beta2 Source Code for PocketGCC 1.50
この文書には[日本語版](readme.md)もあります。

This source code is for **Windows CE (ARMv4I)**.

# Requirements
Make your Windows CE device usable the PocketGCC and a command shell, and use the attached batch file. You may need to edit the batch file depending on your environment.

I use commctrl.lib copied from eMbedded Visual C++ 4.0 to compile. This file is needed to display a command bar, and you have to **get it in the same way or create, use the dynamic linking, or delete the command bar**. (There might be other PocketGCCs that don't require these things.)

This compiler **doesn't support source files includes other than English characters**. If you want to build software including such strings, make them in the String Table of a resource script, or use another compiler like eMbedded Visual C++ 4.0.

# Notes
The source code is "**win.cpp**," the resource script is "**resource.rc**," the icon is "**app.ico**," and the batch file is "**WINBUILD.BAT**."

The executable binary file will be generated as "**AppMain.exe**."