# 素因数分解プログラム w/ Win32 API
This document is also available in [English](README_en.md).

入力された自然数を素因数分解したり、指定範囲で素数を探すシンプルなプログラムです。

対象の数を2以上その数未満のすべての整数で割り、割り切れたときの数を素因数として確定し、割った後の数を再度2以上のすべての整数で割って...を繰り返し、割り切れなくなった時点で最後の素因数と判断して完了する、最も初歩的な素因数分解アルゴリズムに、「**割った結果がその時の割った数より小さくなっていれば、割られた数はその時点で素数と確定できる**」、「**2以外の偶数は素数でない**」という性質を利用して高速化したものです。素数列挙・数え上げ機能でも、同様の高速化を使用しています。

なお、このプログラムには、「**Windows API を用いた GUI アプリケーションのテンプレート**」という意味もあります。Windows API を用いた GUI アプリケーションは作成が非常に大変で、ちょっとした計算や処理をするために数百行もの GUI 関係のコードを記述する必要があります。ここで、様々な機能を取り込んで作成したこのプログラムの計算部分を書き換えれば、別の計算や処理を行わせることができるようになっています。

## 動作要件
### Windows PC 版
**Microsoft Windows XP Service Pack 3 以降 (IA-32/AMD64/AArch32/AArch64)** を推奨

OlderWindows 版は、それ以前の OS でも動作すると思われます。

### Windows CE 版
**SHARP Brain PW-SH1 (Windows Embedded CE 6.0, ARMv5TEJ)** で動作確認を行っています。それ以外の端末は所持しておらず動作確認できていませんが、IDE が対応する CPU 用のコンパイルは行っています。

なお、**SHARP Brain においても新しい機種では実行できない場合があります**。対応機種等の情報は [Brain Wiki](https://brain.fandom.com/ja) 等の Web サイトを参考にしてください。また、日本語で使用する場合は、電子辞書の日本語化が必要です。

## 実行手順
実行ファイルは **[Releases](https://github.com/watamario15/Prime-Factorizarion-Win32API/releases)** から取得できます。

### Windows PC 版
ご使用のコンピュータに合ったものをご利用ください。

|         ファイル名         | 対象コンピュータ |
|:--------------------------:|:----------------:|
|       PF_IA-32.exe         |   32ビット(x86)  |
|       PF_AMD64.exe         |   64ビット(x86)  |
|      PF_AArch32.exe        |   32ビット(ARM)  |
|      PF_AArch64.exe        |   64ビット(ARM)  |
| PF_IA-32(OlderWindows).exe |   32ビット(x86)  |

Avast! アンチウイルスなど、一部のマルウェア対策ソフトではBCCでコンパイルされたプログラムをマルウェアと誤検出する場合があります。**PF_IA-32(OlderWindows).exe** がそれに該当し、誤検出されることがありますが、チェストから復元するなりして実行すれば大丈夫です(心配ならソースコードを確認してください)。

### Windows CE 版
**SHARP Brain** の場合は、本体メモリーや SD カードの**最上層に「アプリ」フォルダ、その中に「素因数分解プログラム」のようなフォルダを作成し、その中に一連のファイルをコピー**することで「追加アプリ・動画」から実行できるようになります (Other CPUs フォルダは不要)。すでに「アプリ」フォルダがある場合は、その中で大丈夫です。適当な位置に配置し、ceOpener や Explorer 等から直接起動することも可能です。

**SHARP Brain 以外の端末**の場合は、ARMv4I と互換性のある端末なら SHARP Brain と同じ実行ファイル (.bin や .cfg は不要)を､別の CPU の端末なら「Other CPUs」フォルダから適切なものを選び、端末所定の方法で実行してください｡なお、製作者は SHARP Brain しか所持していないため、動作確認や動作の保証はできません。

## 使い方
まず起動すると、素因数分解モードになります。メニューバーのオプションから、素数列挙・数え上げの機能に切り替えられます。起動時の言語は、PC 版ではご利用の OS の UI 言語に合わせて日本語か英語が選択されるようになっています。CE 版ではその関数が使えなかったため毎回英語で起動します。オプションの Language から言語の切り替えが可能です。

素数列挙・数え上げ機能は、出力がかなり長くなることがあるため、テキストファイル出力できるようになっています。出力先に既存ファイルが指定された場合、デフォルトではそのファイルの末尾に追記するようになっていますが、この挙動はオプションから変更できます。また、「ファイル」から出力ボックスの内容をテキストファイルに書き出したり、クリップボードにコピーすることができます。なお、こちらのテキストファイル書き出し機能では、**既存ファイルが指定された場合常に上書き**されます。

なお、電子辞書において数値入力は、画面に用意してあるボタンを使っても入力できますし、電子辞書自体のキーボード(アルファベット入力状態のままで OK) でも入力できます。

各機能の詳しい使い方はメニューバーのヘルプから表示できますので、そちらを確認してください。表示中の機能に合わせて、表示されるヘルプが切り替わるようになっています。

## インストール・アンインストールについて
このソフトウェアはインストール不要です。削除も、そのまま実行ファイルを削除するだけで大丈夫です。レジストリなどは使用していません。

## ソースコードについて
各プロジェクトフォルダの readme を参照してください。

## ライセンス
アイコン (app.ico) 以外は MIT License を適用しますが、アイコンを含めた場合は **CC-BY-SA 3.0 License** となります。この際、このアイコンを埋め込んだ exe ファイルも CC-BY-SA 3.0 License となることに注意してください。詳細は [LICENSE](LICENSE) ファイルを確認してください。

## 変更履歴
### v2.1 (2020/10/31)
- エディットボックスへの入力周りに存在した問題を修正した。
- その他、細かい部分を修正した。

### v2 (2020/9/28)
- 正式リリース

### v2β3 (2020/9/19)
- 起動時のチェックの入れ方がおかしかったのを修正。
- 個数のみ表示モードで中断されたとき、その時点で見つかった個数を表示するように変更。
- その他、細かい部分を修正・調整した。

### v2β2 (2020/9/15)
- ソースコード内の変数名などを分かりやすくした。その他、分かりにくくなっていた部分を直した。
- マルチバイトビルドと Unicode ビルドの両方に対応できるコードにした。
- 日本語と英語の両対応にした。それに伴い、文字列はリソースの String Table に移動した。
- 「編集」メニューを追加し、切り取り、コピー、貼り付け、全選択機能を追加した。Ctrl+A での全選択にも対応した。
- 計算処理スレッドの優先順位を、PC 版でも1段階下げた (CE 版では元からこの設定にしてあった)。
- その他、細かい部分を修正・調整した。

### v2β (2020/8/30)
- 全体的にコードを見直した。可読性の向上も図った(つもり)。
- 素数列挙・数え上げの機能を追加した。
- 結果ボックスの内容を、テキストファイルに書き出せるようにした。
- 機能ごとに簡単なヘルプを用意した。
- 画面や文などを全体的に整理した。

### v1.02 (2020/3/27) // Windows CE 版のみ
- コマンドバー (WinCE におけるメニューバー)を追加した

### v1.01 (2020/3/19)
- 素因数分解アルゴリズムを少し改良・修正
- 基本的に入力ボックスにフォーカスが当たるように改良
- 入力ボックスで Enter を押すと計算が実行されるように改良
- 画面の数字キーを押すとカーソルの位置に数字が入力されるよう変更 (WinCE)
- 諸ツール未導入Brainでも物理キーボード入力ができるよう改良 (WinCE)
- 英語版の文を少し変更 (WinCE)
- ARMv4I 以外もとりあえずビルドしてみた (WinCE)

### v1 (2019/5/24)
- 初回リリース