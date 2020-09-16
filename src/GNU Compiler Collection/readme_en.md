# Prime Factorization Software Ver. 2 beta2 Source Code for GNU Compiler Collection
This source code is for **Windows PC**.

# Requirements
Set up the GCC and use the attached batch file to build. You may need to edit the GCC path depending on your environment.

I use **MinGW(gcc 9.2.0)** for the 32-bit version and **MinGW_w64(gcc 8.1.0)** for the 64-bit version. I recommend you to use this version or later.

# Notes
The source code is "**1.cpp**," the resource script is "**resource.rc**," the icon is "**app.ico**," and the batch files are "**build_win32.bat**" and "**build_win64.bat**." These batch files share the same commands, but it becomes easier to build a 32-bit and 64-bit version by setting each of them the proper path.

The executable binary file will be generated as "**PF_(Target CPU).exe**" when you use the attached batch files.