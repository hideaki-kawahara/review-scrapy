
= 最初のスクレイピング

ヤフーニュースをスクレイピングしてみます。


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
Spiderの雛形が作られたらitems.pyを編集します。ファイルがある場所は@<code>{scrapy-source/yahoo_news_scrapy/yahoo_news_scrapy/items.py}です。

こちらはSpiderが出力するアイテムを設定するところになります。下のようにYahooNewsScrapyItemのところを編集します。

//list[After][編集後のitems.py][python]{
import scrapy


class YahooNewsScrapyItem(scrapy.Item):
    url = scrapy.Field()
    title = scrapy.Field()
//}

次はキャッシュの設定をします。

== キャッシュの設定

キャッシュの設定するためにsettings.pyを編集します。これはSpiderが出力するアイテムを設定します。
ファイルがある場所は@<code>{scrapy-source/yahoo_news_scrapy/yahoo_news_scrapy/settings.py}です。

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
Spiderであるyahoo_news.pyを編集します。
ファイルがある場所は@<code>{scrapy-source/yahoo_news_scrapy/yahoo_news_scrapy/spiders/yahoo_news.py}です。

編集内容は下のとおりになります。

 1. 上で作成したitems.pyをimportします。
 2. start_urlsを取得したいURLに変更します。@<href>{https://news.yahoo.co.jp/topics/business, https://news.yahoo.co.jp/topics/business}に変更します。
 3. liタグにニュース記事の一覧が入っているので、response.css('li.newsFeed_item')でピックアップしてfor文で回します。
 4. 記事をピックアップしてyieldを使用してitemsに出力します。
 5. urlはitems.css('a.newsFeed_item_link::attr(href)')にあるので、extract_firstでピックアップします。
 6. titleはitems.css('div.newsFeed_item_title::text')にあるので、でピックアップします。
 7. すべてをピックアップできたらページ遷移をするので、liタグにあるクラスpagination_item-nextにあるaタグをピックアップし、URLを補完して再帰呼び出しを行います。

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
                url=items.css('a.newsFeed_item_link::attr(href)')@<br>{}.extract_first(),
                title=items.css('div.newsFeed_item_title::text')@<br>{}.extract_first()
            )

        next_link = response.css('li.pagination_item-next a::attr(href)')@<br>{}.extract_first()
        if next_link is None:
            return
        yield scrapy.Request(response.urljoin(next_link)@<br>{}, callback=self.parse)
//}

=== 解説

nameとallowed_domainsは自動生成されています。

nameはSpiderの名前でクロールするときに指定する名前です。
allowed_domainsは指定されたドメインのみで動くための設定で、リンクをたどっていくと指定されたドメイン以外にスクレイピングすることを防止します。@<br>{}

start_urlsはスクレイピングするURLを配列で指定します。@<br>{}
デフォルトではstart_urlsで指定されたURLをスクレイピングしてparse関数に引き渡します。

parse関数ではresponse変数を受け取り、ここにスクレイピングした情報などが入っています。@<br>{}

@<code>{response.css}はセレクターになります。他にも@<code>{response.xpath}もセレクターがあり、どちらも情報を取得するための命令になります。

今回の例ではニュース情報は、class指定されているnewsFeed_itemのliタグをすべて取得します。

@<code>{response.css}で抽出されたのを順次@<code>{yield}命令を使用して@<code>{YahooNewsScrapyItem}の中ではurlとtitleの変数が指定されており、そこから順次データを引き渡します。@<br>{}

@<code>{a.newsFeed_item_link::attr(href)}は、classに指定されているnewsFeed_item_linkのaタグを情報を取得します。そしてattr(href)が指定されているのでhrefの情報だけが取得できます。@<br>{}

@<code>{div.newsFeed_item_title::text}、classに指定されているnewsFeed_item_titleのdivタグを情報を取得します。そしてtextが指定されているのでdivタグで囲われている情報だけが取得できます。@<br>{}

@<code>{extract_first()}の指定がないときはcssのレスポンスが返るので指定することで文字列として取得します。

すべてをピックアップしたら、次ページに遷移するので、@<code>{li.pagination_item-next a::attr(href)}は、classに指定されているpagination_item-nextのaタグを情報を取得します。そしてattr(href)が指定されているのでhrefの情報だけが取得できます。@<br>{}

@<code>{response.urljoin}にてhrefの情報を絶対Pathに変換し、再帰呼び出しで次のページへ遷移します。@<br>{}

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
