# Prime Factorization Software's Source Code for MinGW_w64
この文書には[日本語版](readme.md)もあります。

This source code is for **Windows PC**.

## Building Requirements
Set up MinGW\_w64 and build with the included batch file. I recommend you to use GCC 9.3.0 or later.

## Notes
The source code is "**1.cpp**," the resource script is "**resource.rc**," the icon is "**app.ico**," the manifest is **HighDPI.manifest,** and the batch files are "**build_win32.bat**" and "**build_win64.bat**." These batch files share the same commands, but it becomes easier to build a 32-bit and 64-bit version by setting each of them the proper path.

The executable binary file will be generated as "**PF_(Target CPU Architecture).exe**" when you use the attached batch files.