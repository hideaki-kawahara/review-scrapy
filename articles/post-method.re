
= POSTメソッドがあるサイトでスクレイピング
この章では、POSTメソッドで画面遷移をしているサイトでスクレイピングする方法を紹介します。

POSTメソッドは投稿フォームの完了画面やショッピングカートのような、URLをブックマークされると少々困るところで使用することが多く、単なる画面の切り替えやページネーションの画面遷移などでは使われなくなりました。

2010年ごろは、何でもPOSTメソッドで画面遷移するサイトが存在しました。Strutsフレームワークが代表的で、2020年も政府系のサイトでは非常に多く使われおり、Struts2の脆弱性問題で少し減りましたが、システム移行をしてないところが残っております。また、これらのサイトはスマートフォンに対応しておらず、文字コードもShiftJISで表示されていることが多いです。


それでは、POSTメソッドでスクレイピングするには、どうすれば良いのでしょうか？実際にやってみましょう。


あわせて、この章ではクローリングするときにパラメーターを指定して、スクレイピングする情報を変化させてみます。

対象サイトは「国土交通省の賃貸住宅管理業者」URL:@<href>{https://etsuran.mlit.go.jp/TAKKEN/chintaiKensaku.do, https://etsuran.mlit.go.jp/TAKKEN/chintaiKensaku.do}です。


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
Spiderの雛形が作られたらitems.pyを編集します。ファイルがある場所は@<code>{scrapy-source/mlit_scrapy/mlit_scrapy/items.py}です。

こちらはSpiderが出力するアイテムを設定するところになります。下のようにMlitScrapyItemのところを編集します。

//list[After][編集後のitems.py][python]{
import scrapy


class MlitScrapyItem(scrapy.Item):
    licence = scrapy.Field()
    company = scrapy.Field()
    address = scrapy.Field()
//}

次はキャッシュの設定をします。

== キャッシュの設定

キャッシュの設定するためにsettings.pyを編集します。これはSpiderが出力するアイテムを設定します。
ファイルがある場所は@<code>{scrapy-source/mlit_scrapy/mlit_scrapy/settings.py}です。

下のようにHTTPCACHE_ENABLEDのところからコメントアウトされているので、コメントアウトを解除するように編集します。

//list[HTTPCACHE][キャッシュの設定][python]{
HTTPCACHE_ENABLED = True
HTTPCACHE_EXPIRATION_SECS = 0
HTTPCACHE_DIR = 'httpcache'
HTTPCACHE_IGNORE_HTTP_CODES = []
HTTPCACHE_STORAGE = 'scrapy.extensions.httpcache.FilesystemCacheStorage'
//}


次はSpiderを作成します。


== Spider作成
Spiderであるetsuran_mlit.pyを編集します。
ファイルがある場所は@<code>{scrapy-source/mlit_scrapy/mlit_scrapy/spiders/etsuran_mlit.py}です。

編集内容は下のとおりになります。

 1. 上で作成したitems.pyをimportします。@<code>{from mlit_scrapy.items import MlitScrapyItem}
 2. start_urlsを取得したいURLに変更します。@<href>{https://etsuran.mlit.go.jp/TAKKEN/chintaiKensaku.do, https://etsuran.mlit.go.jp/TAKKEN/chintaiKensaku.do}に変更します。
 3. デフォルトの都道府県を設定しておきます。@<code>{pref = '11'}
 4. __init__関数を定義します。
 5. 最初の検索を行う@<code>{scrapy.FormRequest.from_response}でPOST Methodを記載します。
 7. trタグに情報一覧が入っているので、response.css('tr')でピックアップしてfor文で回します。
 8. enumerateとfor文で、tdタグの内容をextract_firstでピックアップし、配列に入れます。
 9. licenceは配列1番目にあるので、itemに出力します。
 10. companyは配列2番目にあるので、itemに出力します。
 11. addressは配列5番目にあるので、itemに出力します。
 12. すべてをピックアップできたらページ遷移をするので、imgタグにonclick属性が登場するので登場したら、フォームデータのCMDにnextを設定して、POST Methodを実行し、@<code>{after_parse}関数をコールバックします。

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
                    formdata = dict( kenCode = self.pref, sortValue = '1', choice = '1'
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

        next_page = response.css('img[src="/TAKKEN/images/result_move_r.jpg"]::attr(onclick)').extract_first()
        if next_page == '':
            return

        yield scrapy.FormRequest.from_response(
                    response,
                    formdata = dict( kenCode = self.pref, sortValue = '1', choice = '1'
                    ,dispCount = '50' ,CMD = 'next'),
                    callback = self.after_parse
                )
//}

=== 解説

ここでの肝は@<code>{scrapy.FormRequest.from_response}です。



== クローラーの実行
Spiderを作成し各種設定をしたら、クローラーを実行します。 DEBUGのところでurlとtitleがピックアップされていることが確認できます。
//list[crawl][クローラーの実行][bash]{
scrapy crawl yahoo_news
//}

== ソースコードについて
この章で使用したソースコードはGitHubにあります。@<href>{https://github.com/hideaki-kawahara/scrapy-source/tree/chapter2, https://github.com/hideaki-kawahara/scrapy-source/tree/chapter2}

 1. Cloneする。
 2. chapter1をcheckoutする。
 2. 仮想環境を作成し、仮想環境に入る。
 3. ライブラリーをインストールする。@<code>{pip install -r requirements.txt}
 4. 該当のディレクトーに入る。
 5. 実行する。@<code>{scrapy crawl yahoo_news}