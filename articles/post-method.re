= POSTメソッドがあるサイトでスクレイピング
この章では、POSTメソッドで画面遷移をしているサイトでスクレイピングする方法を紹介します。@<br>{}

POSTメソッドは投稿フォームの完了画面やショッピングカートのような、URLをブックマークされると少々困るところで使用することが多く、単なる画面の切り替えやページネーションの画面遷移などでは使われなくなりました。@<br>{}

2010年ごろは、何でもPOSTメソッドで画面遷移するサイトが存在しました。Strutsフレームワークが代表的で、2020年も政府系のサイトでは非常に多く使われおり、Struts2の脆弱性問題で少し減りました。

また、2020年になってもシステム移行をしてないところが残っており、これらのサイトはスマートフォンに対応しておらず、文字コードもShiftJISで表示されていることが多いです。@<br>{}


あわせて、この章ではクローリングするときにパラメーターを指定して、スクレイピングする情報を変化させてみます。@<br>{}

対象サイトは「国土交通省の賃貸住宅管理業者」です。

URL:@<href>{https://etsuran.mlit.go.jp/TAKKEN/chintaiKensaku.do, https://etsuran.mlit.go.jp/TAKKEN/chintaiKensaku.do}@<br>{}

5段階評価で難易度を記載します。「国土交通省の賃貸住宅管理業者」サイトの難易度は星2つです。

難易度：★★

== プロジェクトの作成
まずはプロジェクトを作成します。下のコマンドを実行するとmlit_scrapyというディレクトリーが作成されます。

//list[startproject][Projectの作成][bash]{
scrapy startproject mlit_scrapy
//}

ここで作成されるのはScrapyの雛形だけで、Spiderの雛形は作られていません。

次はSpiderの雛形を作成します。


//list[genspider][Spiderの作成][bash]{
cd mlit_scrapy
scrapy genspider etsuran_mlit etsuran.mlit.go.jp
//}



== アイテム設定
Spiderの雛形が作られたらitems.pyを編集します。編集するファイルは@<code>{scrapy-source/mlit_scrapy/mlit_scrapy/items.py}です。


こちらはSpiderが出力するアイテムを設定するところになります。下のようにMlitScrapyItemのところを編集します。

//list[After][編集後のitems.py][python]{
import scrapy


class MlitScrapyItem(scrapy.Item):
    licence = scrapy.Field()
    company = scrapy.Field()
    address = scrapy.Field()
//}

次にディレイタイムの設定とキャッシュの設定をしますが、@<hd>{first-step|ディレイタイムの設定}@<hd>{first-step|キャッシュの設定}と同様に設定しておきます

次はSpiderを作成します。


== Spider作成
Spiderであるetsuran_mlit.pyを編集します。編集するファイルは@<code>{scrapy-source/mlit_scrapy/mlit_scrapy/spiders/etsuran_mlit.py}です。

できたものは下のようになります。

//list[EtsuranMlitSpider][Spiderの編集][python]{
import scrapy
from mlit_scrapy.items import MlitScrapyItem


class EtsuranMlitSpider(scrapy.Spider):
    name = 'etsuran_mlit'
    allowed_domains = ['etsuran.mlit.go.jp']
    start_urls = ['https://etsuran.mlit.go.jp/TAKKEN/chintaiKensaku.do']
    pref = '11'

    def __init__(self, pref='11', *args, **kwargs):
        super(EtsuranMlitSpider, self).__init__(*args, **kwargs)
        self.pref = pref

    def parse(self, response):
        return scrapy.FormRequest.from_response(
                    response,
                    formdata = dict(kenCode = self.pref
                    ,sortValue = '1', choice = '1'
                    ,dispCount = '50' ,CMD = 'search'),
                    callback = self.after_parse
                )

    def after_parse(self, response):
        for tr in response.css('tr'):
            if len(tr.css('td')) != 0:
                items = [''] * 6
                for index, td in enumerate(tr.css('td')):
                    items[ index ] = td.css('::text').extract_first()

                yield MlitScrapyItem(
                    licence = items[ 1 ],
                    company = items[ 2 ],
                    address = items[ 5 ]
                )

        tag = 'img[src="/TAKKEN/images/result_move_r.jpg"]::attr(onclick)'
        next_page = response.css(tag).extract_first()
        if next_page == '':
            return

        yield scrapy.FormRequest.from_response(
                    response,
                    formdata = dict(kenCode = self.pref
                    ,sortValue = '1', choice = '1'
                    ,dispCount = '50' ,CMD = 'next'),
                    callback = self.after_parse
                )
//}

=== 解説
Spiderのソースコードを解説します。

前の章と同じようにスクレイピングしますが、この章での肝は@<code>{scrapy.FormRequest.from_response}で、この命令を使いPOSTメソッドでリクエストします。

//emlist[][python]{
from mlit_scrapy.items import MlitScrapyItem
//}
上で作成したitems.pyをimportしています。

//emlist[][python]{
name = 'etsuran_mlit'
//}
nameはSpiderの名前でクロールするときに指定する名前です。

//emlist[][python]{
allowed_domains = ['etsuran.mlit.go.jp']
//}
allowed_domainsは指定されたドメインのみで動くための設定で、リンクをたどっていくと指定されたドメイン以外にスクレイピングすることを防止します。

//emlist[][python]{
start_urls = ['https://etsuran.mlit.go.jp/TAKKEN/chintaiKensaku.do']
pref = '11'
//}
start_urlsはスクレイピングするURLを配列で指定します。デフォルトではstart_urlsで指定されたURLをスクレイピングしてparse関数に引き渡します。
prefは下で使用するデフォルトの地域を設定しています。

//emlist[][python]{
def __init__(self, pref='11', *args, **kwargs):
    super(EtsuranMlitSpider, self).__init__(*args, **kwargs)
    self.pref = pref
//}
@<code>{-a pref=11}と起動オプションを指定するときに取得する決まった書き方です。取得したパラメーターを他の関数で使用するのでインスタンス変数に代入しています。


//emlist[][python]{
return scrapy.FormRequest.from_response(
            response,
            formdata = dict(kenCode = self.pref
            ,sortValue = '1', choice = '1'
            ,dispCount = '50' ,CMD = 'search'),
            callback = self.after_parse
        )
//}

parse関数ではresponse変数を受け取りスクレイピングしたい情報などが入っていますが、POSTメソッドでアクセスするため使用のみで、@<code>{from_response}を発行します。

POSTするためのデータは@<code>{formdata}で辞書形式にて作成します。このサイトではサーチ結果画面はCMDにsearchを入れると動作します。他にも必要な情報を入れておきます。


//emlist[][python]{
items = [''] * 6
for index, td in enumerate(tr.css('td')):
    items[ index ] = td.css('::text').extract_first()
//}
スクレイピングしたい情報はテーブルタグの中に入っているのですが、登録番号（licence）以外はclassを設定していないので@<code>{enumerate}を使用して配列に入れておくとあとで処理がしやすいです。あとは欲しい配列インデックスを指定してデータを引き引き渡します。


//emlist[][python]{
tag = 'img[src="/TAKKEN/images/result_move_r.jpg"]::attr(onclick)'
next_page = response.css(tag).extract_first()
if next_page == '':
    return

yield scrapy.FormRequest.from_response(
            response,
            formdata = dict(kenCode = self.pref
            ,sortValue = '1', choice = '1'
            ,dispCount = '50' ,CMD = 'next'),
            callback = self.after_parse
        )
//}

すべてをピックアップしたら、次ページに遷移するので、次のページのリンク情報を取得します。

次ページを示すimgタグにonclick属性が設定されるので、確認できたらCMDにnextを設定して次ページへ遷移するというフォームデータを作成し、再帰呼び出しでPOSTメソッドを発行して次のページへ遷移します。


== クローラーの実行
Spiderを作成し各種設定をしたら、クローラーを実行します。 DEBUGのところで情報（licence,address,title）がピックアップされていることが確認できます。
//list[crawl][クローラーの実行][bash]{
scrapy crawl etsuran_mlit
//}

Spiderに渡すパラメーターは@<code>{-a}で指定します。東京都は@<code>{-a pref=13}と指定します。10以下の道県は0を加えて指定します。
//list[crawl_option][クローラーを実行するときのオプション指定][bash]{
scrapy crawl etsuran_mlit -a pref=13
scrapy crawl etsuran_mlit -a pref=01
//}

== ソースコードについて
この章で使用したソースコードはGitHubにあります。

@<href>{https://github.com/hideaki-kawahara/scrapy-source/tree/chapter2, https://github.com/hideaki-kawahara/scrapy-source/tree/chapter2}

実行する手順を下に記載します。

//emlist[][bash]{
 1. ソースコードをCloneするディレクトリーを作成する。
   @<code>{mkdir -p scrapy-source}
 2. Cloneする。
   @<code>{git clone https://github.com/hideaki-kawahara/scrapy-source.git}
 3. chapter2をcheckoutする。
   @<code>{git checkout chapter2}
 4. 仮想環境を作成する。
   @<code>{python -m venv .venv}
 5. 仮想環境に入る。
   @<code>{source .venv/bin/activate}
 6. ライブラリーをインストールする。
   @<code>{pip install -r requirements.txt}
 7. 該当のディレクトリーに入る。
   @<code>{cd mlit_scrapy}
 8. 実行する。
   @<code>{scrapy crawl etsuran_mlit}
//}

※実行後に実行キャッシュディレクトリーが作成されるので、他のBrunchをcheckoutしてもchapter2のディレクトリーは消えません。気になるようなら削除してください。

@<code>{rm -rf mlit_scrapy}
