# 素因数分解プログラム MinGW\_w64 版ソースコード
This document is also available in [English](readme_en.md).

**Windows PC** 用のソースコードです。

## ビルド時の要件
MinGW\_w64 を使える環境を整え、ビルド用のバッチファイルでビルドしてください。GCC 9.3.0 以降のバージョンを推奨します。

## 補足
ソースコードは **1.cpp**、リソーススクリプトは **resource.rc**、アイコンは **app.ico**、マニフェストは **HighDPI.manifest** です。**build_win32.bat** と **build_win64.bat** がビルド用のバッチファイルです。コマンドは共通ですので、適切にパスを振り分けてご利用ください。

実行可能バイナリは、ビルド用バッチファイルを用いた場合は **PF_(対象CPUアーキテクチャ名).exe** として生成されるようになっています。