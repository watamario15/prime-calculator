# Prime Factorization Software's Source Code for BCC Developer
この文書には[日本語版](readme.md)もあります。

This source code is for **Windows PC**.

## Building Requirements
This project is for **BCC Developer**, and I use **Borland C++ Compiler 5.5** as a compiler. You must edit the Include Path in "**Project Settings->Resources**" to fit your environment. This compiler is quite old and supports Windows XP and older, and can't compile 64-bit applications.

## Notes
The source code is "**1.cpp**", the resource script is "**resource.rc**", and the icon is "**app.ico**". You can edit and build this project by opening "**PrimeFactorization.bdp**" if you have BCC Developer. To avoid errors, don't put this folder on a path that includes non-ASCII characters like Japanese characters.

The executable binary file will be generated as "**PF_IA-32(OlderWindows).exe**" in the "**Debug**" folder for debug builds, and the "**Release**" folder for release builds.