# 素因数分解プログラム Ver. 2β2 BCC Developer版ソースコード
**Windows PC**用のソースコードです。

# ビルド時の要件
このプロジェクトは、**BCC Developer**で作成されています。また、製作者はコンパイラとして**Borland C++ Compiler 5.5**を使用しています。

なお、このコンパイラは超古いのでWindows XPにも普通に対応している他、64bitプログラムのビルドは行えません。

# 注釈
ソースコードは**1.cpp**、リソーススクリプトは**resource.rc**、アイコンは**app.ico**です。BCC Developerであれば、**PrimeFactorization.bdp**を開くことでプログラムの編集及びビルドが可能です(日本語を含むパスに配置するとビルド時に問題が発生することがあるので、極力避けてください)。

実行可能バイナリは、デバッグビルドは**Debug**フォルダ、リリースビルドは**Release**フォルダに**PF_IA-32(OlderWindows).exe**として生成されます。