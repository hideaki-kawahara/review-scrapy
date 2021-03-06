
= 環境構築
今回使用するScrapyフレームワークはPythonで動作します。
そのためPythonを使用できるようにします。著者の環境はバージョン3.9.1を使用しますが、3系であれば問題ないです。

2系は日本語処理に癖がある、将来的にサポート外になるなどの問題があることから筆者としてはオススメしておりません。

== macOSでのPython

macOSではPythonがインストールされていますが、macOS 11 Big Surで入っているPythonのバージョンは2.7.16で、冒頭でも書いたとおり2系になるので3系に変更します。

@<href>{https://www.python.org/, https://www.python.org/} で配布しているインストーラーでも、pyenvやPipenvなどの管理ツールなど、またはHomebrewに慣れている方はHomebrewを使用してください。Anaconda系が入っている方は変更してくださいとは言いませんが、余計なツールを入れてしまうなどがあるので、新規に入れるのはオススメしておりません。

上のインストールツールを使用してPythonの3系をインストールします。

== Windows 10でのPython

Windows 10ではPythonが入っておりません。

macOSと同じく@<href>{https://www.python.org/, https://www.python.org/} で配布しているインストーラーで良いです。もしWSL2を利用しているなら、その環境にPythonの3系は自動的に入ります。それとmacOSと同じようにAnaconda系はオススメしません。

上のインストールツールを使用してPythonの3系をインストールします。

またWindows 10はGitが入っていないのでGitも合わせてインストールします。

== 環境を作る
ソースコードを置くところを作成してから仮想環境の構築を行います。
//list[venv][仮想環境構築][bash]{
mkdir -p scrapy-source
python -m venv .venv
//}

仮想環境の作成ができたら環境を切り替えます。

macOSやWSL2では、以下の方法で切り替えます。
//list[mac][macOSやWSL2での環境切り替え][bash]{
source .venv/bin/activate
//}

コマンドプロンプトでは、以下の方法で切り替えます。
//list[dos][コマンドプロンプトでの環境切り替え][bash]{
.venv\Scripts\activate.bat
//}

PowerShellでは、以下の方法で切り替えます。

※実行許可がないときは、実行許可を与えてください。
//list[ps1][PowerShellでの環境切り替え][bash]{
.venv\Scripts\activate.ps1
//}


仮想環境を構築したら、Scrapyフレームワークをインストールします。
//list[pip][Scrapyフレームワークのインストール][bash]{
pip install scrapy
//}

これで環境ができました。あとは好きなエディターで読み込ませれば完了です。

なお、この書籍で使用した筆者作成のソーリコードはGitHubに公開しています。

@<href>{https://github.com/hideaki-kawahara/scrapy-source.git, https://github.com/hideaki-kawahara/scrapy-source.git}

== Docker
この書籍ではDockerを使うことを前提で記載しています。
Dockerの知識は必須ではないですが、Dockerのインストールと起動する知識があると良いです。

次のようにコマンドラインを叩いて、dockerとdocker-composeのバージョンが出るようにDockerを準備してください。
//list[docker][Dockerのバージョン確認について][bash]{
docker version
docker-compose version
//}
