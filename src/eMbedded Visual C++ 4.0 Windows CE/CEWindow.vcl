<html>
<body>
<pre>
<h1>ビルドのログ</h1>
<h3>
--------------------構成 : CEWindow - Win32 (WCE x86) Release--------------------
</h3>
<h3>コマンド ライン</h3>
Creating command line "rc.exe /l 0x411 /fo"X86Rel/resource.res" /d UNDER_CE=400 /d _WIN32_WCE=400 /d "UNICODE" /d "_UNICODE" /d "NDEBUG" /d "WCE_PLATFORM_STANDARDSDK" /d "_X86_" /d "x86" /d "_i386_" /r "F:\Programming\eMbedded Visual C++ 4.0 Windows CE\resource.rc"" 
一時ファイル "C:\DOCUME~1\WATA\LOCALS~1\Temp\RSP322.tmp" を作成し、次の内容を記録します
[
/nologo /W3 /Od /D _WIN32_WCE=400 /D "WCE_PLATFORM_STANDARDSDK" /D "_i386_" /D UNDER_CE=400 /D "i_386_" /D "UNICODE" /D "_UNICODE" /D "_X86_" /D "x86" /D "NDEBUG" /D "IA32" /Fp"X86Rel/CEWindow.pch" /YX /Fo"X86Rel/" /Gs8192 /GF /c 
"F:\Programming\eMbedded Visual C++ 4.0 Windows CE\1.cpp"
]
Creating command line "cl.exe @C:\DOCUME~1\WATA\LOCALS~1\Temp\RSP322.tmp" 
一時ファイル "C:\DOCUME~1\WATA\LOCALS~1\Temp\RSP323.tmp" を作成し、次の内容を記録します
[
commctrl.lib coredll.lib corelibc.lib /nologo /base:"0x00010000" /stack:0x10000,0x1000 /entry:"WinMainCRTStartup" /incremental:no /pdb:"X86Rel/PF_x86.pdb" /nodefaultlib:"OLDNAMES.lib" /nodefaultlib:libc.lib /nodefaultlib:libcd.lib /nodefaultlib:libcmt.lib /nodefaultlib:libcmtd.lib /nodefaultlib:msvcrt.lib /nodefaultlib:msvcrtd.lib /out:"X86Rel/PF_x86.exe" /subsystem:windowsce,4.00 /MACHINE:IX86 
".\X86Rel\1.obj"
".\X86Rel\resource.res"
]
コマンド ライン "link.exe @C:\DOCUME~1\WATA\LOCALS~1\Temp\RSP323.tmp" の作成中
<h3>アウトプット ウィンドウ</h3>
リソースをコンパイル中...
コンパイル中...
1.cpp
リンク中...




<h3>結果</h3>
PF_x86.exe - エラー 0、警告 0
</pre>
</body>
</html>
