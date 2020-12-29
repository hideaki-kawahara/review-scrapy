
= 最初のスクレイピング

ヤフーニュースをスクレイピングしてみます。

対象サイトは「ヤフーニュース」のビジネスページです。

URL:@<href>{https://news.yahoo.co.jp/topics/business, https://news.yahoo.co.jp/topics/business}@<br>{}

5段階評価で難易度を記載します。ヤフーニュースの難易度は1つです。

難易度：★

== プロジェクトの作成
まずはプロジェクトを作成します。下のコマンドを実行するとyahoo_news_scrapyというディレクトリーが作成されます。

//list[startproject][Projectの作成][bash]{
scrapy startproject yahoo_news_scrapy
//}

ここで作成されるのはScrapyの雛形だけで、Spiderの雛形は作られていません。

次はSpiderの雛形を作成します。


//list[genspider][Spiderの作成][bash]{
cd yahoo_news_scrapy
scrapy genspider yahoo_news news.yahoo.co.jp
//}

== アイテム設定
Spiderの雛形が作られたらitems.pyを編集します。設定するファイルは@<code>{scrapy-source/yahoo_news_scrapy/yahoo_news_scrapy/items.py}です。

こちらはSpiderが出力するアイテムを設定するところになります。下のようにYahooNewsScrapyItemのところを編集します。

//list[After][編集後のitems.py][python]{
import scrapy


class YahooNewsScrapyItem(scrapy.Item):
    url = scrapy.Field()
    title = scrapy.Field()
//}

次はディレイタイムの設定をします。

== ディレイタイムの設定
ディレイタイムの設定します。

これはSpiderが次のアクションに入るまでの待ち時間を設定します。編集するファイルは@<code>{scrapy-source/yahoo_news_scrapy/yahoo_news_scrapy/settings.py}です。

DOWNLOAD_DELAYがコメントアウトされているので、下のようにコメントアウトを解除するように編集します。


//list[DOWNLOAD_DELAY][ディレイタイムの設定][python]{
DOWNLOAD_DELAY = 3
//}

次はキャッシュの設定をします。

== キャッシュの設定

キャッシュの設定します。

これはSpiderがスクレイピングするサイトをキャッシュする設定をします。編集するファイルは@<code>{scrapy-source/yahoo_news_scrapy/yahoo_news_scrapy/settings.py}です。

HTTPCACHE_ENABLEDのところからコメントアウトされているので、下のようにコメントアウトを解除するように編集します。

//list[HTTPCACHE][キャッシュの設定][python]{
HTTPCACHE_ENABLED = True
HTTPCACHE_EXPIRATION_SECS = 0
HTTPCACHE_DIR = 'httpcache'
HTTPCACHE_IGNORE_HTTP_CODES = []
HTTPCACHE_STORAGE = 'scrapy.extensions.httpcache.FilesystemCacheStorage'
//}


次はSpiderを作成します。


== Spider作成
Spiderであるyahoo_news.pyを編集します。編集するファイルは@<code>{scrapy-source/yahoo_news_scrapy/yahoo_news_scrapy/spiders/yahoo_news.py}です。

できたものは下のようになります。


//list[YahooNewsSpider][Spiderの編集][python]{
import scrapy
from yahoo_news_scrapy.items import YahooNewsScrapyItem

class YahooNewsSpider(scrapy.Spider):
    name = 'yahoo_news'
    allowed_domains = ['news.yahoo.co.jp']
    start_urls = ['https://news.yahoo.co.jp/topics/business']

    def parse(self, response):
        for items in response.css('li.newsFeed_item'):
            yield YahooNewsScrapyItem(
                url = items.css('a.newsFeed_item_link::attr(href)').extract_first(),
                title = items.css('div.newsFeed_item_title::text').extract_first()
            )

        next_link = response.css('li.pagination_item-next a::attr(href)').extract_first()
        if next_link is None:
            return
        yield scrapy.Request(response.urljoin(next_link), callback=self.parse)
//}

=== 解説
Spiderのソースコードを解説します。

Scrapyがスクレイピングした内容を、セレクターを使用して欲しい情報を抽出し、それをアイテム情報に設定します。現在のページを抽出し終わったら、次のページに画面遷移を行い同じことを繰り返します。次のページに画面遷移が行えなくなったら処理を終了します。


//emlist[][python]{
from yahoo_news_scrapy.items import YahooNewsScrapyItem
//}
上で作成したitems.pyをimportしています。

//emlist[][python]{
name = 'yahoo_news'
//}
nameはSpiderの名前でクロールするときに指定する名前です。

//emlist[][python]{
allowed_domains = ['news.yahoo.co.jp']
//}
allowed_domainsは指定されたドメインのみで動くための設定で、リンクをたどっていくと指定されたドメイン以外にスクレイピングすることを防止します。

//emlist[][python]{
start_urls = ['https://news.yahoo.co.jp/topics/business']
//}
start_urlsはスクレイピングするURLを配列で指定します。デフォルトではstart_urlsで指定されたURLをスクレイピングしてparse関数に引き渡します。@<br>{}


parse関数ではresponse変数を受け取り、ここにスクレイピングしたい情報などが入っています。@<br>{}

@<code>{response.css}はセレクターになります。他にも@<code>{response.xpath}もセレクターがあり、どちらも情報を取得するための命令になります、今回は理解しやすい@<code>{response.css}セレクターを使用します。

//emlist[][python]{
for items in response.css('li.newsFeed_item'):
    yield YahooNewsScrapyItem(
        url = items.css('a.newsFeed_item_link::attr(href)').extract_first(),
        title = items.css('div.newsFeed_item_title::text').extract_first()
    )
//}

今回の例ではニュース情報は、class指定されているnewsFeed_itemのliタグをすべて取得します。liタグにニュース記事の一覧が入っているので、@<code>{response.css('li.newsFeed_item')}でピックアップしてfor文で回します。

@<code>{response.css}で抽出されたのを順次@<code>{yield}命令を使用して@<code>{YahooNewsScrapyItem}の中ではurlとtitleの変数が指定されており、そこから順次データを引き渡します。@<br>{}

urlとして取得するのは@<code>{a.newsFeed_item_link::attr(href)}になります。classに指定されている@<code>{newsFeed_item_link}のaタグを情報を取得します。そして@<code>{attr(href)}が指定されているのでaタグのhref情報だけが取得できます。@<br>{}

titleとして取得するのは@<code>{div.newsFeed_item_title::text}になります。classに指定されている@<code>{newsFeed_item_title}のdivタグを情報を取得します。そして@<code>{::text}が指定されているのでdivタグで囲われているテキスト情報だけが取得できます。

//emlist[][python]{
next_link = response.css('li.pagination_item-next a::attr(href)').extract_first()
if next_link is None:
    return
yield scrapy.Request(response.urljoin(next_link), callback=self.parse)
//}

すべてをピックアップしたら、次ページに遷移するので、次のページのリンク情報を取得します。@<code>{li.pagination_item-next a::attr(href)}になります。classに指定されている@<code>{pagination_item-next}のaタグを情報を取得します。そして@<code>{attr(href)}が指定されているのでaタグのhref情報だけが取得できます。@<br>{}

次ページのリンク情報取得したら@<code>{response.urljoin}にてhrefの情報を絶対Pathに変換し、再帰呼び出しで次のページへ遷移します。

== クローラーの実行
Spiderを作成し各種設定をしたら、クローラーを実行します。 DEBUGのところでurlとtitleがピックアップされていることが確認できます。
//list[crawl][クローラーの実行][bash]{
scrapy crawl yahoo_news
//}

csv形式で出力も可能です。
//list[crawl_csv][クローラーの実行、CSVで出力][bash]{
scrapy crawl yahoo_news -o yahoo_news.csv
//}

指定可能な拡張子は下の通りで、拡張子を指定することでファイル形式を指定します。

 * csv
 * json
 * jsonlines
 * jl
 * marshal
 * pickle
 * xml

拡張子をjsonにするとjson形式に、jsonlinesにすると見やすいjson形式になります。

他にもJuliaで使用するjl形式や、PythonやRubyのオブジェクトのシリアライズで使うmarshal形式やpickle形式、Javaで有名になったxml形式があります。
プラグインを開発すれば、自分で好きな出力形式を作ることも可能です。

== ソースコードについて
この章で使用したソースコードはGitHubにあります。

@<href>{https://github.com/hideaki-kawahara/scrapy-source/tree/chapter1, https://github.com/hideaki-kawahara/scrapy-source/tree/chapter1}

実行する手順を下に記載します。

 1. ソースコードをCloneするディレクトリーを作成する。@<br>{}
   @<code>{mkdir -p scrapy-source}
 2. Cloneする。@<br>{}
   @<code>{git clone https://github.com/hideaki-kawahara/scrapy-source.git}
 3. chapter1をcheckoutする。@<br>{}
   @<code>{git checkout chapter1}
 4. 仮想環境を作成する。@<br>{}
   @<code>{python -m venv .venv}
 5. 仮想環境に入る。@<br>{}
   @<code>{source .venv/bin/activate}
 6. ライブラリーをインストールする。@<br>{}
   @<code>{pip install -r requirements.txt}
 7. 該当のディレクトーに入る。@<br>{}
   @<code>{cd yahoo_news_scrapy}
 8. 実行する。@<br>{}
   @<code>{scrapy crawl yahoo_news}

※実行後に実行キャッシュディレクトリーが作成されるので、他のBrunchをcheckoutしてもchapter1のディレクトリーは消えません。気になるようなら削除してください。

@<code>{rm -rf yahoo_news_scrapy}
