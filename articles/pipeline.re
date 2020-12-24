
= PipelineとORM

Qiitaのトレンドをスクレイピングして分析してみます。

分析することで言語やフレームワークなどのトレンドが日によって変化することが見えてきます。

== ORMの導入
ローカル環境で動かしますが、ORM（オブジェクト関係マッピング、Object-Relational Mapping）を使用してデータベースにアクセスします。

今回はORMとしてSqlalchemyを利用します。環境構築@<chap>{building-environmen}をしたあとにSqlalchemyをインストールします。
//list[sqlalchemy][sqlalchemyの用意][bash]{
pip install sqlalchemy
//}


== データベースの準備
次にデータベースを用意します。
ローカル環境にデータベースを入れるか、Dockerでデータベースを入れてください。


今回はDockerでデータベースを入れてみます。
Dockerの入れ方は、この書籍では記載しませんが、dockerとdocker-composeを事前にインストールしておきます。@<br>{}

Docker用のディレクトリーを作成します。
//list[docker][Docker用ディレクトリー作成][bash]{
mkdir -p docker
cd docker
//}

Docker用ディレクトリーにdocker-compose.ymlファイルを作成します。

今回はPostgreSQLを使います。

//list[After][docker-compose.ymlファイルの内容][yml]{
version: '3'
services:
  postgres:
    image: postgres:latest
    container_name: scrapy_postgres
    environment:
      POSTGRES_HOST_AUTH_METHOD: 'trust'
    volumes:
      - ./.db/data:/usr/local/var/postgres
      - ./initdb:/docker-entrypoint-initdb.d
    ports:
      - 5432:5432
//}

次にデータベースの初期設定ファイルを作成します。
//list[initdb][Docker用ディレクトリー作成][bash]{
mkdir -p initdb
cd initdb
//}

初期設定用ディレクトリーにsetup.sqlファイルを作成します。

//list[setup.sql][setup.sqlファイルの内容][sql]{
create table tags
(
    id INTEGER UNIQUE NOT NULL,
    keyword TEXT NOT NULL,
    count INTEGER NOT NULL,
    create_at timestamp with time zone default CURRENT_TIMESTAMP,
    update_at timestamp with time zone default CURRENT_TIMESTAMP,
    PRIMARY KEY (id)
);
//}

dockerディレクトリーに戻り、Dockerコンテナーを起動します。
//list[docker-compose][Dockerコンテナーを起動][bash]{
cd ..
docker-compose up -d
//}

起動しているのを確認するにはDockerコマンドを叩きます。
//list[docker exec][PostgreSQLの起動確認][bash]{
docker exec -i -t scrapy_postgres bash
su postgres
psql -l
//}

下のようにデータベースのリストが表示されたらDockerコンテナーは起動しています。
//indepimage[list_of_database]

== プロジェクトの作成
次にプロジェクトを作成します。Dockerディレクトーから上の階層に戻り、下のコマンドを実行するとqiita_trend_scrapyというディレクトリーが作成されます。

//list[startproject][Projectの作成][bash]{
cd ..
scrapy startproject qiita_trend_scrapy
//}

ここで作成されるのはScrapyの雛形だけで、Spiderの雛形は作られていません。

次はSpiderの雛形を作成します。


//list[genspider][Spiderの作成][bash]{
cd qiita_trend_scrapy
scrapy genspider qiita_trend qiita.com
//}


== アイテム設定
Spiderの雛形が作られたらitems.pyを編集します。ファイルがある場所は@<code>{scrapy-source/yahoo_news_scrapy/items.py}です。

こちらはSpiderが出力するアイテムを設定するところになります。下のようにYahooNewsScrapyItemのところを編集します。

//list[item][編集後のitems.py][python]{
import scrapy


class YahooNewsScrapyItem(scrapy.Item):
    url = scrapy.Field()
    title = scrapy.Field()
//}

次はキャッシュの設定をします。

== キャッシュの設定

キャッシュの設定するためにsettings.pyを編集します。これはSpiderが出力するアイテムを設定します。
ファイルがある場所は@<code>{scrapy-source/yahoo_news_scrapy/settings.py}です。

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
ファイルがある場所は@<code>{scrapy-source/yahoo_news_scrapy/spiders/yahoo_news.py}です。

編集内容は下のとおりになります。

 1. 上で作成したitems.pyをimportします。
 2. start_urlsを取得したいURLに変更します。@<href>{https://news.yahoo.co.jp/topics/it, https://news.yahoo.co.jp/topics/it}に変更します。
 3. liタグにニュース記事の一覧が入っているので、response.css('li.newsFeed_item')でピックアップしてfor文で回します。
 4. 記事をピックアップしてyieldを使用してitemsに出力します。
 5. urlはitems.css('a.newsFeed_item_link::attr(href)')にあるので、extract_firstでピックアップします。
 6. titleはitems.css('div.newsFeed_item_title::text')にあるので、でピックアップします。

できたものは下のようになります。


//list[YahooNewsSpider][Spiderの編集][python]{
import scrapy
from yahoo_news_scrapy.items import YahooNewsScrapyItem

class YahooNewsSpider(scrapy.Spider):
    name = 'yahoo_news'
    allowed_domains = ['news.yahoo.co.jp']
    start_urls = ['https://news.yahoo.co.jp/topics/it']

    def parse(self, response):
        for items in response.css('li.newsFeed_item'):
            yield YahooNewsScrapyItem(
                url=items.css('a.newsFeed_item_link::attr(href)')@<br>{}.extract_first(),
                title=items.css('div.newsFeed_item_title::text')@<br>{}.extract_first()
            )
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


== pipeline


== クローラーの実行
Spiderを作成し各種設定をしたら、クローラーを実行します。 DEBUGのところでurlとtitleがピックアップされていることが確認できます。
//list[crawl][クローラーの実行][bash]{
scrapy crawl yahoo_news
//}







データベースを落とすときは下のコマンドを叩きます。

//list[docker down][Dockerの落とし方][bash]{
cd docker
docker-compose down
//}

