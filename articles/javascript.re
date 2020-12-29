
= 環境構築


== JavaScriptレンダリングの導入
ローカル環境で動かしますが、ORM（オブジェクト関係マッピング、Object-Relational Mapping）を使用してデータベースにアクセスします。

今回はORMとしてSplashを利用します。環境構築@<chap>{building-environmen}をしたあとにSqlalchemyをインストールします。
//list[scrapy-splash][Splashの用意][bash]{
pip install scrapy-splash
//}


== Splashの準備
SplashをDockerで準備します。

Dockerの入れ方は、この書籍では記載しませんが、dockerとdocker-composeを事前にインストールしておきます。@<br>{}

Docker用のディレクトリーを作成します。
//list[docker][Docker用ディレクトリー作成][bash]{
mkdir -p docker
cd docker
//}

Docker用ディレクトリーにdocker-compose.ymlファイルを作成します。

//list[After][docker-compose.ymlファイルの内容][yml]{
version: '3'
services:
  scrapinghub:
    image: scrapinghub/splash:latest
    container_name: scrapinghub
    environment:
      TZ: Asia/Tokyo
    ports:
      - 8050:8050
//}

Dockerコンテナーを起動します。
//list[docker-compose][Dockerコンテナーを起動][bash]{
docker-compose up -d
//}


== プロジェクトの作成
次にプロジェクトを作成します。Dockerディレクトーから上の階層に戻り、下のコマンドを実行するとqiita_trend_scrapyというディレクトリーが作成されます。

//list[startproject][Projectの作成][bash]{
cd ..
scrapy startproject techbookfest_scrapy
//}

ここで作成されるのはScrapyの雛形だけで、Spiderの雛形は作られていません。

次はSpiderの雛形を作成します。


//list[genspider][Spiderの作成][bash]{
cd techbookfest_scrapy
scrapy genspider techbookfest_url techbookfest.org
//}


== アイテム設定
Spiderの雛形が作られたらitems.pyを編集します。

ファイルがある場所は@<code>{scrapy-source/qiita_trend_scrapy/qiita_trend_scrapy/items.py}です。

こちらはSpiderが出力するアイテムを設定するところになります。下のようにQiitaTrendScrapyItemのところを編集します。

//list[item][編集後のitems.py][python]{
import scrapy


class TechbookfestScrapyItem(scrapy.Item):
    title = scrapy.Field()
    digital = scrapy.Field()
    paper = scrapy.Field()
    url = scrapy.Field()
//}

次はディレイタイムの設定をします。

== ディレイタイムの設定
ディレイタイムの設定するためにsettings.pyを編集します。これはSpiderが出力するアイテムを設定します。

ファイルがある場所は@<code>{scrapy-source/qiita_trend_scrapy/qiita_trend_scrapy/settings.py}です。

下のようにDOWNLOAD_DELAYがコメントアウトされているので、コメントアウトを解除するように編集します。


//list[DOWNLOAD_DELAY][キャッシュの設定][python]{
DOWNLOAD_DELAY = 3
//}

次はキャッシュの設定をします。

== キャッシュの設定
キャッシュの設定するためにsettings.pyを編集します。これはSpiderが出力するアイテムを設定します。

ファイルがある場所は@<code>{scrapy-source/qiita_trend_scrapy/qiita_trend_scrapy/settings.py}です。

下のようにHTTPCACHE_ENABLEDのところからコメントアウトされているので、コメントアウトを解除するように編集します。

//list[HTTPCACHE][キャッシュの設定][python]{
HTTPCACHE_ENABLED = True
HTTPCACHE_EXPIRATION_SECS = 0
HTTPCACHE_DIR = 'httpcache'
HTTPCACHE_IGNORE_HTTP_CODES = []
HTTPCACHE_STORAGE = 'scrapy.extensions.httpcache.FilesystemCacheStorage'
//}

次はDownload Middlewareの設定をします。





== Download Middlewareの設定
Splashを利用するためにはDownload Middlewareの設定します。


SPIDER_MIDDLEWARES = {
   'techbookfest_scrapy.middlewares.TechbookfestScrapySpiderMiddleware': 543,
}


ファイルがある場所は@<code>{scrapy-source/qiita_trend_scrapy/qiita_trend_scrapy/settings.py}です。

下のようにDOWNLOADER_MIDDLEWARESのところからコメントアウトされているので、コメントアウトを解除するように編集し、scrapy_splashの部分を追記します。DOWNLOADER_MIDDLEWARESの値が750なので、scrapy_splashは少し小さい値に設定します。

そして、その下にJavaScriptレンダリングが起動しているURLを@<code>{SPLASH_URL = 'http://localhost:8050/'}指定します。


//list[DOWNLOADER_MIDDLEWARES][pipelineの設定][python]{
DOWNLOADER_MIDDLEWARES = {
    'scrapy_splash.SplashCookiesMiddleware': 723,
    'scrapy_splash.SplashMiddleware': 725,
    'techbookfest_scrapy.middlewares.TechbookfestScrapyDownloaderMiddleware': 810,
}
SPLASH_URL = 'http://localhost:8050/'
DUPEFILTER_CLASS = 'scrapy_splash.SplashAwareDupeFilter'
HTTPCACHE_STORAGE = 'scrapy_splash.SplashAwareFSCacheStorage'
//}


次はSpiderを作成します。

== Spider作成
Spiderであるqiita_trend.pyを編集します。
ファイルがある場所は@<code>{scrapy-source/qiita_trend_scrapy/qiita_trend_scrapy/spiders/qiita_trend.py}です。

編集内容は下のとおりになります。

 1. 上で作成したitems.pyをimportします。
 2. Qiitaタグはaタグのcss-4czcteのclassに入っているので、response.css('a.css-4czcte')でピックアップしてfor文で回します。
 3. Qiitaタグをカウントするので、Dictを使いカウントします。
 4. カウントが終わったらアイテムとして出力します。

できたものは下のようになります。

https://scrapbox.io/wakaba-manga/

//list[QiitaTrendSpider][Spiderの編集][python]{
import scrapy
import scrapy_splash
from scrapbox_scrapy.items import ScrapboxScrapyItem

LUA_SCRIPT = """
function main(splash)
    splash.private_mode_enabled = false
    splash:go(splash.args.url)
    splash:wait(1)
    html = splash:html()
    return html
end
"""


class ScrapboxUrlSpider(scrapy.Spider):
    name = 'scrapbox_url'
    allowed_domains = ['scrapbox.io']
    start_url = ''

    def __init__(self, *args, **kwargs):
        super(ScrapboxUrlSpider, self).__init__(*args, **kwargs)


    def start_requests(self):
        if self.start_url != '':
            yield scrapy_splash.SplashRequest(
                self.start_url,
                self.parse,
                endpoint='execute',
                args={'lua_source': LUA_SCRIPT},
            )

    def parse(self, response):
        for uls in response.css('ul.page-list'):
            for hrefs in uls.css('a'):
                yield ScrapboxScrapyItem(
                    title = hrefs.css('a::text').extract_first(),
                    url = response.urljoin(hrefs.css('a::attr(href)').extract_first())
                )
//}

次はPipelineを作成します。

== Pipeline
Pipelineでは受け取ったitemに対して処理をします。

ファイルがある場所は@<code>{scrapy-source/qiita_trend_scrapy/qiita_trend_scrapy/pipeline.py}です。

Tagテーブル情報のインスタンスを用意し、@<code>{session.add}でレコードを追加して@<code>{session.commit}すると、データベースに記録されていきます。


//list[QiitaTrendScrapyPipeline][Pipelineの編集][python]{
from itemadapter import ItemAdapter
from . database import session ,Base
from . tag import Tag


class QiitaTrendScrapyPipeline:
    def process_item(self, item, spider):
        tags = Tag()
        tags.keyword = item['keyword']
        tags.count = item['count']

        session.add(tags)
        session.commit()


        return item
//}


== クローラーの実行
Spiderを作成し各種設定をしたら、クローラーを実行します。 DEBUGメッセージにSqlalchemyの実行結果が表示されていることが確認できます。
//list[crawl][クローラーの実行][bash]{
scrapy crawl qiita_trend
//}

== データベースの確認
実行が終わったらデータの確認をします。データベースに入って確認します。

Dockerに入り、SQLを叩きます。

//list[result][データベース登録確認][bash]{
docker exec -i -t scrapy_postgres bash
su postgres
psql
select * from tags order by count desc;
//}

下のように表示されたら正しく実行されています。
//indepimage[result2]


確認が終わり、データベースを落とすときは下のコマンドを叩きます。

//list[docker down][Dockerの落とし方][bash]{
cd docker
docker-compose down
//}







