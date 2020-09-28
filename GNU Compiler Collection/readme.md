# 素因数分解プログラム Ver. 2 GNU Compiler Collection版ソースコード
This document is also available in [English](readme_en.md).

**Windows PC**用のソースコードです。

# ビルド時の要件
GCCを使える環境を整えたうえで、ビルド用のバッチファイルを環境に合わせて書き換えてビルドしてください(特にパス周り)。

製作者は、32bit版を **MinGW(gcc 9.2.0)** 、64bit版を **MinGW_w64(gcc 8.1.0)** でビルドしていますので、これ以降のバージョンでビルドすることをお勧めします。


# 注釈
ソースコードは**1.cpp**、リソーススクリプトは**resource.rc**、アイコンは**app.ico**です。**build_win32.bat**と**build_win64.bat**がビルド用のバッチファイルです。コマンドは共通ですので、適切にパスを振り分けてご利用ください。

実行可能バイナリは、ビルド用バッチファイルを用いた場合は **PF_(対象CPUアーキテクチャ名).exe** として生成されるようになっています。