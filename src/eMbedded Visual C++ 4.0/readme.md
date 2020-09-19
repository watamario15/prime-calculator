# 素因数分解プログラム Ver. 2β3 eMbedded Visual C++ 4.0版ソースコード
This document is also available in [English](readme_en.md).

**Windows CE**用のソースコードです。

# ビルド時の要件
このプロジェクトは、**eMbedded Visual C++ 4.0**でビルド可能です。[開発環境構築 | Brain Wiki | Fandom](https://brain.fandom.com/ja/wiki/%E9%96%8B%E7%99%BA%E7%92%B0%E5%A2%83%E6%A7%8B%E7%AF%89)などを参考にしてインストールしてください。

ただし、このIDEはWindows 2000とXPでしか動かないという代物です。

# 注釈
ソースコードは**1.cpp**、リソーススクリプトは**resource.rc**、アイコンは**app.ico**です。eMbedded Visual C++ 4.0であれば、**PrimeFactorization.vcw**を開くことでプログラムの編集及びビルドが可能です。

実行可能バイナリは、デバッグビルドは**ARMV4IDbg**フォルダ、リリースビルドは**ARMV4IRel**フォルダに**AppMain.exe**として生成されます。

なお、その他CPU用のバイナリも **「(CPU名)Dbg」** 、 **「(CPU名)Rel」** の中に **「PF_(CPU名).exe」** として生成されます｡