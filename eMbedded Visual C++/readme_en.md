# Prime Factorization Software's Source Code for eMbedded Visual C++ 4.0
この文書には[日本語版](readme.md)もあります。

This source code is for **Windows CE**.

## Building Requirements
You can build this project with **eMbedded Visual C++ 4.0**. Note that this IDE only supports Windows 2000 and Windows XP.

## Notes
The source code is "**1.cpp**", the resource script is "**resource.rc**", and the icon is "**app.ico**". You can edit and build this project by opening "**PrimeFactorization.vcw**" if you have eMbedded Visual C++ 4.0.

The ARMv4I executable binary file will be generated as "**AppMain.exe**" in the "**ARMV4IDbg**" folder for debug builds, and the "**ARMV4IRel**" folder for release builds. Files for other CPUs will be generated as "**PF_(Target CPU Architecture).exe**" in "**(Target CPU Architecture)Dbg**" folder for debug builds, and "**(Target CPU Architecture)Rel**" folder for release builds.