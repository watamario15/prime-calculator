# 素因数分解プログラム eMbedded Visual C++ 4.0 版ソースコード
This document is also available in [English](readme_en.md).

**Windows CE** 用のソースコードです。

## ビルド時の要件
このプロジェクトは、**eMbedded Visual C++ 4.0** でビルド可能です。[Brain Wiki](https://brain.fandom.com/ja/wiki/Microsoft_eMbedded_Visual_C%2B%2B_4.0) などを参考にしてインストールしてください。ただし、この IDE は互換モードを使わなければ Windows 2000 と XP でしか動かないという代物です。

## 補足
ソースコードは **1.cpp**、リソーススクリプトは **resource.rc**、アイコンは **app.ico** です。eMbedded Visual C++ 4.0 であれば、**PrimeFactorization.vcw** を開くことでプログラムの編集及びビルドが可能です。

実行可能バイナリは、デバッグビルドは **ARMV4IDbg** フォルダ、リリースビルドは **ARMV4IRel** フォルダに **AppMain.exe** として生成されます。なお、その他CPU用のバイナリも **「(CPU名)Dbg」** 、 **「(CPU名)Rel」** の中に **「PF_(CPU名).exe」** として生成されます｡