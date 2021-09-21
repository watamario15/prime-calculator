# 素因数分解プログラム PocketGCC 1.50 版ソースコード
This document is also available in [English](readme_en.md).

**Windows CE (StrongARM)** 用のソースコードです。

## ビルド時の要件
Windows CE 端末に、[Brain Wiki](https://brain.fandom.com/ja/wiki/Brain%E3%81%A7%E3%83%97%E3%83%AD%E3%82%B0%E3%83%A9%E3%83%9F%E3%83%B3%E3%82%B0) 等を参考にして PocketGCC/DOS窓Open を使える状態にし、付属の bat ファイル(環境に合わせて特にパス周りを書き換える)を用いてビルドしてください。

なお、製作者はコンパイル時に eMbedded Visual C++ 4.0 の Standard SDK からコピーした commctrl.lib をリンカに読み込ませています。これはコマンドバーの表示に必要なものですので、**同様の方法で commctrl.lib を入手するか、ダイナミックリンクを使うか、コマンドバーを削除するか**を行ってください(別版の PocketGCC で、この操作が不要なものがあるかもしれません)。

このコンパイラでは**日本語を含むプログラムをコンパイルできません**。日本語で作成したい場合は、日本語文字をリソーススクリプトに移動するか、eMbedded Visual C++ 4.0 や CeGCC などの別のコンパイラを使用してください。

## 補足
ソースコードは **win.cpp**、リソーススクリプトは **resource.rc**、アイコンは **app.ico**、ビルド用バッチファイルは **WINBUILD.BAT** です。実行可能バイナリは **AppMain.exe** として生成されます。