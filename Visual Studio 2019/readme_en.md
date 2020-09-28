# Prime Factorization Software Ver. 2 Source Code for Visual Studio 2019
この文書には[日本語版](readme.md)もあります。

This source code is for **Windows PC**.

# Requirements
This project has been created by **Visual Studio Enterprise 2019**.

This project uses the **v141_xp build set, which supports Windows XP**, and **ARM and ARM64** are enabled. You have to install these build sets first. If you haven't installed yet, Visual Studio will prompt you to install them. (You can build for x64 and Win32, without ARM or ARM64 build tools.)

If you don't want to or can't install them, you can disable the Windows XP support by the following procedure. 

>Open "**Project -> Properties**", and switch the "**Construction**" to "**All Constructions**" and the "**Platform**" to "**All Platforms**". Then, change the following items.
>
>1. Change "**General -> Platform Toolset**" to "**Visual Studio 2019 (v142)**"\*.
>1. Press OK and close. Then, reopen the properties page.
>1. "**Change General -> Windows SDK Version**" to "**10.0 (Latest installed version)**"\*.
>1. "**Change C/C++ -> Language -> Permissive Mode**" to "**True**".

\*This is a setting for Visual Studio 2019 as of August 2020. Maybe it works on other versions if you choose the latest one.

\*Since I use the Japanese version of Visual Studio, the name of the items might be different.

# Notes
The source code is "**1.cpp**," the resource script is "**resource.rc**," and the icon is "**app.ico**". You can edit and build by opening "**PrimeFactorization.sln**" if you use Visual Studio.

Executable binary files will be generated as "**PF_(Target CPU).exe**" in the "**Release / Debug**" folder in "**Win32**" for x86(32-bit), "**x64**" for x86(64-bit), "**ARM**" for ARM(32-bit), and "**ARM64**" for ARM(64-bit). The "**Release**" folder is for release builds, and "**Debug**" folder is for debug builds.