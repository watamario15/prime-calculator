# Prime Factorizarion Software w/ Win32 API
A simple free software performs calculations related to prime numbers. Using Win32 API.

# このプログラムについて

入力された自然数を素因数分解したり、指定範囲で素数を探すシンプルなプログラムです。

対象の数を2以上その数未満のすべての整数で割り、割り切れたときの数を素因数として確定し、割った後の数を再度2以上のすべての整数で割って...を繰り返し、割り切れなくなった時点で最後の素因数と判断して完了する、最も初歩的な素因数分解アルゴリズムに、「**割った結果がその時の割った数より小さくなっていれば、割られた数はその時点で素数と確定できる**」、「**2以外の偶数は素数でない**」という性質を利用して高速化したものです。

素数列挙・数え上げ機能でも、同様の高速化を使用しています。

なお、このプログラムには、「**Windows APIを用いたGUIアプリケーションのテンプレート**」という意味もあります。Windows APIを用いたGUIアプリケーションは作成が非常に大変で、ちょっとした計算や処理をするために数百行ものGUI関係のコードを記述する必要があります。

ここで、様々な機能を取り込んで作成したこのプログラムの計算部分を書き換えれば、別の計算や処理を行わせることができるようになっています。

# 動作要件
## Windows PC版
**Microsoft Windows XP Service Pack 3以降(x86/AMD64/AArch32/AArch64)を推奨**

OlderWindows版は、それ以前のOSでも動作すると思われます。

## Windows CE版
**SHARP Brain PW-SH1 (Windows Embedded CE 6.0, ARMV4I)** で動作確認を行っています。それ以外の端末は所持しておらず動作確認できていませんが、IDEが対応するCPU用のコンパイルは行っています。

なお、**SHARP Brainにおいても新しい機種では実行できない場合があります**。対応機種等の情報は[Brain Wiki](https://brain.fandom.com/ja)等のWebサイトを参考にしてください。また、日本語版を使用する場合は、電子辞書の日本語化が必要です。

# 実行手順
実行ファイルは、 **「bin_(対象端末)」** にあります。
## Windows PC版
ご使用のコンピューターに合ったものをご利用ください。
- PF_IA-32.exe : 32ビット(x86)
- PF_AMD64.exe : 64ビット(x86)
- PF_AArch32.exe : 32ビット(ARM)
- PF_AArch64.exe : 64ビット(ARM)
- PF_IA-32(OlderWindows).exe : 32ビット(x86) <- 上記の実行ファイルが使えない古いPC用

Avast!アンチウイルスなど、一部のマルウェア対策ソフトではBCCでコンパイルされたプログラムをマルウェアと誤検出する場合があります。**PF_IA-32(OlderWindows).exe**がそれに該当し、誤検出されることがありますが、チェストから復元するなりして実行すれば大丈夫です(心配ならソースコードを確認してください)。

## Windows CE版
**SHARP Brain**の場合は、内蔵メモリーやSDカードの**最上層に「アプリ」フォルダを作成して、一連のファイルをコピー**することで「追加アプリ・動画」から実行できるようになります。すでに「アプリ」フォルダがある場合は、その中にコピーしてください。適当な位置に配置し、ceOpenerやExplorer等から起動することも可能です。

**SHARP Brain以外の端末**の場合は、ARMv4Iの端末ならSHARP Brainと同じファイルを､別のCPUの端末なら「Other CPU」フォルダから適切なものを選び、端末所定の方法で実行してください｡なお、製作者はSHARP Brainしか所持していないため、動作確認や動作の保証はできません。

# 使い方
まず起動すると、素因数分解モードになります。

メニューバーのオプションから、素数列挙・数え上げの機能に切り替えられます。

素数列挙・数え上げ機能は、出力がかなり長くなることがあるため、テキストファイル出力できるようになっています。出力先に既存ファイルが指定された場合、デフォルトではそのファイルの最後から追記するようになっていますが、この挙動はオプションから変更できます。

また、「ファイル」から出力ボックスの内容をテキストファイルに書き出したり、クリップボードにコピーすることができます。なお、こちらのテキストファイル書き出し機能では、**既存ファイルが指定された場合常に上書き**されます。

なお、電子辞書において数値入力は、画面に用意してあるボタンを使っても入力できますし、電子辞書自体のキーボード(アルファベット入力状態のままでOK)でも入力できます。

各機能の詳しい使い方は、メニューバーのヘルプから表示できますので、そちらを確認してください。表示中の機能に合わせて、表示されるヘルプが切り替わるようになっています。

# インストール・アンインストールについて
このソフトウェアはインストール不要です。削除も、そのまま実行ファイルを削除するだけで大丈夫です。レジストリなどは使用していません。

# ソースコードについて
ソースコードは **「src」** にあります。
ビルド方法などは各プロジェクトフォルダのreadmeを参照してください。

# 使用上の注意
製作者は、このプログラムの利用によって生じた、いかなる損害についても責任を負いません。

# 著作権情報など
このソフトウェアはフリーソフトウェアですが、製作者(watamario)は著作権を放棄しておりませんので、**無断転載は禁止**です。しかし、このプログラムをテンプレートとして作成した全く別のソフトウェアに関しては、製作者が著作権を主張することはございませんので、自由に制作・配布いただいて構いません。

ただし、**アイコンに[Brain Wiki](https://brain.fandom.com/ja)にあった画像を編集したもの**が含まれています。配布時には注意してください。

# 変更履歴
## v2β(2020/8/30)

全体的にコードを見直した。可読性の向上も図った(つもり)。

素数列挙・数え上げの機能を追加した。

結果ボックスの内容を、テキストファイルに書き出せるようにした。

機能ごとに簡単なヘルプを用意した。

画面や文などを全体的に整理した。

## v1.02(2020/3/27) // Windows CE版のみ

コマンドバー(WinCEにおけるメニューバー)を追加した

## v1.01(2020/3/19)

素因数分解アルゴリズムを少し改良・修正

基本的に入力ボックスにフォーカスが当たるように改良

入力ボックスでEnterを押すと計算が実行されるように改良

画面の数字キーを押すとカーソルの位置に数字が入力されるよう変更

諸ツール未導入Brainでも物理キーボード入力ができるよう改良

英語版の文を少し変更

ARMv4I以外もとりあえずビルドしてみた

## v1(2019/5/24)

初回リリース
