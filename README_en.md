# Prime Factorization Software w/ Win32 API
この文書には[日本語版](README.md)もあります。

This software factors a natural number into prime numbers and looks up prime numbers in a specified range.

First, the most simple prime factorization algorithm can be described as:
1. Try to divide the input natural number by all integers between 1 and that number, then judge the first number that could divide the input natural number is a prime factor. If no such number found, then the input number is a prime number.
1. Then, repeat this process with the result of the division until it becomes a prime number.

This software accelerates the algorithm by using the following properties (Trial division):
- **If a result of a division is smaller than the divisor, you can decide that a dividend is a prime number.**
- **Even numbers other than 2 are not prime numbers.**

I used a similar algorithm in the "List/Count Prime Numbers" feature.

By the way, you can use this program as a **template for creating your own GUI application w/ Windows API**. It's quite troublesome to create GUI applications w/ Windows API on your own, and I made it easier by providing this program. **You can create one by just replacing the functions that perform calculations!**

## System Requirements
### Windows PC
**Microsoft Windows XP Service Pack 3 or newer (IA-32/AMD64/AArch32/AArch64)** is recommended.

The "OlderWindows" version might work on older Windows versions.

### Windows CE
I tested on **SHARP Brain PW-SH1 (Windows Embedded CE 6.0, ARMv5TEJ**. I compiled for all CPUs that the IDE supports, but I don't test them since I don't have such devices.

## How to run
You can get executable files at the "**[Releases](https://github.com/watamario15/Prime-Factorizarion-Win32API/releases)**" page.

### Windows PC
Use one matches your computer.

|         File Name          | Target Computer |
|:--------------------------:|:---------------:|
|       PF_IA-32.exe         |  32-bit (x86)   |
|       PF_AMD64.exe         |  64-bit (x86)   |
|      PF_AArch32.exe        |  32-bit (ARM)   |
|      PF_AArch64.exe        |  64-bit (ARM)   |
| PF_IA-32(OlderWindows).exe |  32-bit (x86)   |

Some antimalware software like Avast! Antivirus wrongly detects software compiled by Borland C++ Compiler as malware. Since this problem, they might detect "**PF_IA-32(OlderWindows).exe**" as malware. In such cases, please restore from the chest and run. You can check the source code if you desire.

### Windows CE
Use "**AppMain.exe**" for ARMv4I compatible devices, and select an appropriate one from "**Other CPUs**" for other devices. Then, run it in a way that your device requires.

## How to use
First, this software launches with the "Prime Factorization" mode. Enter a natural number in the input box, and press the OK button or the Enter key to start a calculation. Also, You can switch to the "List/Count Prime Numbers" mode in the options menu. Specify a range to search prime numbers and a max count, and press the OK button or the Enter key to start a calculation. Since the output can get long, this feature supports outputting to a text file (the output box has the 65,535 characters limit). You can also use the file menu to export the contents of the output box to a text file or copy it to the clipboard.

The PC version launches with your OS's UI language, but the Windows CE version always use English as default since Windows CE doesn't support the function to get the UI language information. You can switch the language at "Options -> Language."

You can find further instructions at "Help -> How to use." This displays the proper one for the selected mode.

## How to install/uninstall
You don't need to install this software. Just run the executable file directly. Since this software doesn't use the registry or such, you can uninstall by just deleting the files, too.

## About source codes
Please refer to the readmes in each project folder.

## License
Although the MIT License is applied to files other than the application icon (app.ico), the overall license will be the **CC-BY-SA 3.0 License** if you include the icon. Note that executable files with this icon embedded will be licensed under the CC-BY-SA 3.0 License, too. Refer to [LICENSE](LICENSE) for more details.

## Release notes
### v2.1 (10/31/2020)
- Fixed issues around inputting to edit controls.
- Fixed other minor issues.

### v2 (9/28/2020)
- General Release

### v2 beta3 (9/19/2020)
- Fixed the initial checked/unchecked status of the menu.
- When a user interrupted the "List/Count Prime Numbers" feature in the "Display only number of prime numbers" mode, the number of found prime numbers at that time is now shown.
- Fixed other minor issues and usability.

### v2 beta2 (9/15/2020)
- Renamed variables in the source code to improve the readability.
- The source code can now be compiled as both Multibyte and Unicode.
- This software now supports English and Japanese. In this work, I moved strings to the String Table.
- Added the "Edit" menu, and cut, copy, paste, and select all are now supported.
- Calculation threads' priority is now below normal on the PC version, too.
- Fixed other minor issues and usability.

### v2 beta (8/30/2020)
- Refreshed the code. Readability should have been improved.
- Added the List/Count Prime Numbers feature.
- The contents of the output box can now be output to a text file.
- Each feature has a short how-to-use description now.
- Improved the design and descriptions.

### v1.02 (3/27/2020) // Released only for Windows CE
- Added a command bar.

### v1.01 (3/19/2020)
- Improved and fixed the prime factorization algorithm.
- Updated to always set focus on edit controls.
- You can now start the calculation by pressing the Enter key.
- Changed the behavior of the screen keyboard to enter numbers at the cursor location (WinCE).
- You can now enter numbers with qwerty... keys (WinCE).
- Improved English descriptions (WinCE).
- Started to build for CPUs other than ARMv4I. (WinCE)

### v1 (5/24/2019)
- First release.