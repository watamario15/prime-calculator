# Prime Factorization Software w/ Win32 API
この文書には[日本語版](README.md)もあります。

This software performs prime factorization and looking up prime numbers in a specified range.

First, the most simple algorithm is this.
1. Try to divide the input value by all integers between 1 and that number, then judge a number that could firstly divide the number is a prime factor.
1. Then, repeat this procedure with the result of the division until it becomes a prime number.

This software accelerates this algorithm by using the following properties.
- **If a result of a division is smaller than the divisor, you can decide that a dividend is a prime number.**
- **Numbers other than 2 are not a prime number.**

I used a similar algorithm in the "List/Count Prime Numbers" feature.

By the way, you can use this program as a **template to create your own GUI application w/ Windows API**. It's quite troublesome to create GUI applications w/ Windows API on your own, and I made it easier by providing this program. **You can create one by just replacing the functions that perform calculations!**

# System Requirements
## Windows PC
**Microsoft Windows XP Service Pack 3 or newer (IA-32/AMD64/AArch32/AArch64)** is recommended.

The "OlderWindows" version might work on older Windows.

## Windows CE
I tested on **SHARP Brain PW-SH1 (Windows Embedded CE 6.0, ARMV4I)**.

I compiled for CPUs that the IDE supports. However, since I don't have other devices, I can't test them.

# How to run
I put executable files in "bin_(Target Device)".
## Windows PC
Please use one which matches your computer.

|         File Name          | Target Computer |
|:--------------------------:|:---------------:|
|       PF_IA-32.exe         |  32-bit (x86)   |
|       PF_AMD64.exe         |  64-bit (x86)   |
|      PF_AArch32.exe        |  32-bit (ARM)   |
|      PF_AArch64.exe        |  64-bit (ARM)   |
| PF_IA-32(OlderWindows).exe |  32-bit (x86)   |

Some antimalware software like Avast! Antivirus wrongly detects a software compiled by Borland C++ Compiler as a malware. Since this problem, they might detect **PF_IA-32(OlderWindows).exe** as malware. In such cases, please restore from the chest and run. You can check the source code if you desire.

## Windows CE
AppMain.exe is for ARMv4I devices. If your device is other than that, select one from "Other CPU". Then, run it in a way that your device requires.

# How to use
First, this software launches with the Prime Factorization mode.

You can switch to the List/Count Prime Numbers feature in the options menu.

This software launches with your OS's UI language. Since some Windows CE doesn't support the function, Windows CE version software always launches with English. You can switch the language at Options -> Language.

Since the output may get long, the List/Count Prime Numbers feature supports outputting to a text file. As the default setting, this software postscripts if a selected file already exists. You can change this behavior at Options -> Overwrite an existing file.

Also, you can save the contents of the output box as a text file. Note that this feature **always overwrites an existing file**.

You can see other contents at Help -> How to use. This menu displays the proper one for the selected mode.

# How to install / uninstall
You don't need to install this software. Please run the executable file directly. You can also uninstall by just deleting the file. This software doesn't use registry or such.

# About source codes
Source codes are in **src**. Please refer to the readmes in each project folder.

# Notes
**THE AUTHOR OF THIS SOFTWARE WILL NOT TAKE ANY RESPONSIBILITY FOR ANY DAMAGES BY USING THIS SOFTWARE.**

# Rights
This is free software, but **ALL RIGHTS RESERVED**. However, it's free to redistribute your completely original software based on this program.

But, note that **this software includes an icon retrieved and edited from the [Brain Wiki](https://brain.fandom.com/ja)**.

Author: watamario, otwthutu15(AtSign)gmail.com

# Known issues
    While you are entering with IME, the software stops working.
Short term mitigation: **Enter with Alphabet/Number input mode.**

    When you try to paste content from clipboard history in Windows 10, you will no longer be able to operate the software.
Short term mitigation: **Use Ctrl+V. You can use edit controls in other software to the contents in the Clipboard History to be pastable.**

    You can't enter with the new IME included in Windows 10 May 2020 Update.
Short term mitigation: **Enable compatibility mode from the system setting, referring to [this MS page](https://support.microsoft.com/en-us/help/4564002/you-might-have-issues-on-windows-10-version-2004-when-using-some-micro).** (It seems this new IME has numerous problems as of September 2020.)

I'm going to fix these issues when I get to know the workaround. If you know that, help me by telling me!

# Release notes
## v2 beta2 (9/15/2020)
Renamed variables in the source code for readability.

The source code can now be compiled as both Multibyte and Unicode.

This software now supports English and Japanese. In this work, I moved strings to the String Table.

Added the "Edit" menu, and cut, copy, paste and select all are now supported.

Calculation threads' priority is now below normal also on the PC version.

Fixed other minor issues and usability.

## v2 beta (8/30/2020)
Refreshed the code. Readability may have also be improved.

Added the List/Count Prime Numbers feature.

The contents of the output box can now be output to a text file.

Now, each feature has a short how-to-use sentence.

Improved the design and sentences.

## v1.02 (3/27/2020) // Released only for Windows CE
Added a command bar.

## v1.01 (3/19/2020)
Improved and fixed the prime factorization algorithm.

Improved to edit controls get focused.

You can now start the calculation by the Enter key.

Changed the behavior of the screen keyboard to enter numbers where a cursor exists (WinCE).

You can now enter numbers with qwerty... keys (WinCE).

Improved English sentences (WinCE).

Started to build for CPUs other than ARMv4I. (WinCE)

## v1 (5/24/2019)
First release.