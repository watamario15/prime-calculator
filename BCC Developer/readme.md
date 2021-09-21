# 素因数分解プログラム BCC Developer 版ソースコード
This document is also available in [English](readme_en.md).

**Windows PC** 用のソースコードです。

## ビルド時の要件
このプロジェクトは、**BCC Developer** で作成されています。また、製作者はコンパイラとして **Borland C++ Compiler 5.5** を使用しています。また、**プロジェクト設定->リソース** のインクルードパスを環境に合わせて書き換える必要があります。なお、このコンパイラは超古いので Windows XP にも普通に対応している他、64-bit プログラムのビルドは行えません。

## 補足
ソースコードは **1.cpp**、リソーススクリプトは **resource.rc**、アイコンは **app.ico** です。BCC Developer であれば、**PrimeFactorization.bdp** を開くことでプログラムの編集及びビルドが可能です(日本語を含むパスに配置するとビルド時に問題が発生することがあるので、極力避けてください)。

実行可能バイナリは、デバッグビルドは **Debug** フォルダ、リリースビルドは **Release** フォルダに **PF_IA-32(OlderWindows).exe** として生成されます。