# Prime Factorization Software's Source Code for Visual Studio
この文書には[日本語版](readme.md)もあります。

This source code is for **Windows PC**.

## Building Requirements
This project is for **Visual Studio 2019**.

This project uses **v141_xp build tools supporting Windows XP** for Win32/x64, and **ARM and ARM64** are enabled. You must install these build tools first. If you haven't installed them yet, Visual Studio will prompt you to install. (You can still build for x64 and Win32, without ARM/ARM64 build tools.)

If you don't want to or can't install them, you can disable the Windows XP support by the following procedure:
1. Open "**Project**" -> "**Properties**," and switch the "**Configuration**" to "**All Configurations**" and the "**Platform**" to "**All Platforms**."
1. Set the "**General**" -> "**Platform Toolset**" to "**Visual Studio 2019 (v142)**."\*
1. Press the OK button, then reopen the properties page.
1. Set the "**General**" -> "**Windows SDK Version**" to "**10.0 (latest installed version)**."\*
1. Set "**C/C++**" -> "**Language**" -> "**Conformance Mode**" to "**True**."

\* These settings are for Visual Studio 2019 as of September 2021. Choosing the latest one should work when displayed options are different from this instruction.

\* Since I use the Japanese version of Visual Studio, the name of the items might be different.

## Creating the project from scratch
I created this project by the following procedure:
1. Install the "Desktop development with C++" workload, "MSVC v142 - VS 2019 C++ ARM build tools (latest)," "MSVC v142 - VS 2019 C++ ARM64 build tools (latest)," and "C++ Windows XP Support for VS 2017 (v141) tools \[Deprecated\]."
1. Choose the "Windows Desktop Wizard" in the project creation.
1. Set to place the solution and the project at the same place, type the name, and then create.
1. Choose the "Desktop Application (.exe)" and "Blank Project", then press the OK button.
1. Add source codes, resource scripts, etc at the Solution Explorer.
1. Open the "Configuration Manager" at "Project" -> "Properties."
1. Select the "New" at "Platform", select "ARM", select "Win32" to copy settings from, check to create the new solution platform, and then press OK.
1. Add "ARM64" in the same way (I chose the x64 to copy settings from).
1. Set "All Configurations" and "Win32" to configure.
1. At "General" -> "Output/Intermediate directory," change the directory structures to agree with other platforms.
1. Set "All Configurations" and "Multiple Platforms" with Win32 and x64 to configure.
1. Change the tool set to "Visual Studio 2017 - Windows XP (v141\_xp)" and apply.
1. At "C/C++" -> "Language," change C++ Standard to "C++17" and Conformance Mode to "No."
   - The tool set for Windows XP itself isn't conformity with the C++ standard.
1. Set "All Configurations" and "Multiple Platforms" with "ARM" and "ARM64" to configure, then reopen the properties page.
1. Set the "C/C++ Standard" to "C17" and "C++17" respectively.
1. Set "All Configurations" and "ARM" to configure.
1. Type `/D _AArch32` in "C/C++" -> "Command Line" -> "Additional Options."
1. Type `/D _AArch64` for ARM64 in the same way.
1. Set "All Configurations" and "All Platforms" to configure.
1. Change the "C/C++" -> "Command Line" -> "Warning Level" to "Level 4."
1. At "Manifest Tool" -> "Input/Output," set the "Additional Manifest Files" to `HighDPI.manifest` and the "DPI Awareness" to "Per Monitor High DPI Aware."
   - The `HighDPI.manifest` provided with this project is based on the standard PerMonitorV2 configuration, but removed the `<dpiAware>` tag to prevent from the duplication with the autoset manifest by Visual Studio. This manifest file causes the `Unrecognized Element "dpiAwareness"` warning to be displayed while building for Win32/x64 due to the use of the old tool set for Windows XP, but the manifest is embedded correctly and no problem.
1. Change the "Resource" -> "General" -> "Culture" to "Japanese (Japan) (0x411) (/l 0x0411)"
1. Change the "C/C++" -> "Code Generation" -> "Runtime Library" to "Multithreaded Debug (/MTd)" and "Multithreaded (/MT)" respectively for "Debug" and "Release."
   - This forces the compiler to statically link necessary libraries and makes the program runnable on computers without the Visual Studio redistributable package.

## Notes
The source code is "**main.cpp**," the resource script is "**resource.rc**," the icon is "**app.ico**", and the manifest is "**HighDPI.manifest**." You can edit and build this project by opening "**PrimeFactorization.sln**" if you have Visual Studio.

Executable binary files will be generated as "**PF_(Targeted CPU Architecture).exe**" in the "**Release/Debug**" folder in "**Win32**" (x86), "**x64**" (AMD64), "**ARM**" (AArch32), and "**ARM64**" (AArch64). The "**Release**" folder is for release builds, and the "**Debug**" folder is for debug builds.