= 動的画面のスクレイピング

動的画面をスクレイピングします。

動的画面はJavaScriptが画面のレンダリングを完了したあとに、はじめて画面として表示されるため、HTMLを取得するだけではスクレイピングができません。

この場合SeleniumやPuppeteerなどのヘッドレスブラウザでJavaScriptレンダリングを行わせてからスクレイピングします。

今回はJavaScriptレンダリングとしてSplashを利用します。

対象サイトは「Scrapbox」でログイン無しにてアクセスできるページです。

URL:@<href>{https://scrapbox.io/product, https://scrapbox.io/product}@<br>{}

5段階評価で難易度を記載します。「Scrapbox」サイトの難易度は星4つです。

難易度：★★★★


== JavaScriptレンダリングの導入
環境構築@<chap>{building-environmen}をしたあとにScrapy splashをインストールします。
//list[scrapy-splash][Splashの用意][bash]{
pip install scrapy-splash
//}


== Splashの準備
JavaScriptレンダリングとしてSplashをDockerで準備します。

Dockerの入れ方は、この書籍では記載しませんが、dockerとdocker-composeを事前にインストールしておきます。

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

ブラウザを起動して@<href>{http://localhost:8050, http://localhost:8050}につなげると、JavaScriptレンダリングであるSplashのGUIにアクセスできます。

//indepimage[Splash][SplashのGUI]

== プロジェクトの作成
次にプロジェクトを作成します。Dockerディレクトリーから上の階層に戻り、下のコマンドを実行するとscrapbox_scrapyというディレクトリーが作成されます。

//list[startproject][Projectの作成][bash]{
cd ..
scrapy startproject scrapbox_scrapy
//}

ここで作成されるのはScrapyの雛形だけで、Spiderの雛形は作られていません。

次はSpiderの雛形を作成します。


//list[genspider][Spiderの作成][bash]{
cd scrapbox_scrapy
scrapy genspider scrapbox_url scrapbox.io
//}


== アイテム設定
Spiderの雛形が作られたらitems.pyを編集します。編集するファイルは@<code>{scrapbox_scrapy/scrapbox_scrapy/items.py}です。

こちらはSpiderが出力するアイテムを設定するところになります。下のようにScrapboxScrapyItemのところを編集します。

//list[item][編集後のitems.py][python]{
import scrapy


class ScrapboxScrapyItem(scrapy.Item):
    title = scrapy.Field()
    url = scrapy.Field()
//}

次にディレイタイムの設定とキャッシュの設定をしますが、@<hd>{first-step|ディレイタイムの設定}と@<hd>{first-step|キャッシュの設定}と同じになるので、同じように設定しておきます。

次はJavaScriptレンダリング用の設定をします。


== JavaScriptレンダリング用の設定
Splashを利用するためには下の設定します。編集するファイルは@<code>{scrapbox_scrapy/scrapbox_scrapy/items.py}です。

 * Spider Middleware
 * Download Middleware
 * Splash entry URL
 * Splash用DupeFilter
 * Splash用HTTP Cache
 * AutoThrottle

Spider Middlewareのところがコメントアウトされているので、コメントアウトを解除するように編集します。

//list[SPIDER_MIDDLEWARES][Spider Middlewareの設定][python]{
SPIDER_MIDDLEWARES = {
   'techbookfest_scrapy.middlewares.TechbookfestScrapySpiderMiddleware': 543,
}
//}


Download Middlewareのところからコメントアウトされているので、コメントアウトを解除するように編集します。数値はSplashのMiddlewareが標準のDownload Middlewareより優先させたいので750以下になるように設定します。

//list[DOWNLOADER_MIDDLEWARES][Download Middlewareの設定][python]{
DOWNLOADER_MIDDLEWARES = {
    'scrapy_splash.SplashCookiesMiddleware': 723,
    'scrapy_splash.SplashMiddleware': 725,
    'techbookfest_scrapy.middlewares@<embed>$|latex|\linebreak\hspace*{5ex}$.TechbookfestScrapyDownloaderMiddleware': 810,
}
//}

Splash entry URL、Splash用DupeFilter、Splash用HTTP Cacheについては記載が存在しないので下のように追記します。DupeFilterとHTTP Cacheを設定しないと、キャッシュをうまくできなくなるので必ず設定してください。

//list[ETC][各種追記内容][python]{
SPLASH_URL = 'http://localhost:8050/'
DUPEFILTER_CLASS = 'scrapy_splash.SplashAwareDupeFilter'
HTTPCACHE_STORAGE = 'scrapy_splash.SplashAwareFSCacheStorage'
//}

AutoThrottleのところがコメントアウトされているので、コメントアウトを解除するように編集します。ネットワークが遅くなるときにディレイを入れてくれる設定です。

//list[AUTOTHROTTLE][AutoThrottleの設定][python]{
AUTOTHROTTLE_ENABLED = True
AUTOTHROTTLE_START_DELAY = 5
AUTOTHROTTLE_MAX_DELAY = 60
//}


次はSpiderを作成します。

== Spider作成
Spiderであるscrapbox_url.pyを編集します。編集するファイルは@<code>{scrapbox_scrapy/scrapbox_scrapy/spiders/scrapbox_url.py}です。

できたものは下のようになります。

//list[QiitaTrendSpider][Spiderの編集][python]{
import scrapy
import scrapy_splash
from scrapbox_scrapy.items import ScrapboxScrapyItem

LUA_SCRIPT = """
function main(splash)
    splash.private_mode_enabled = false
    splash:go(splash.args.url)
    splash:wait(1)
    splash.private_mode_enabled = true
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
                    url = response.urljoin(hrefs.css('a::attr(href)')
                    .extract_first())
                )

//}

=== 解説
Spiderのソースコードを解説します。

他の章と同じようにスクレイピングします。

//emlist[][python]{
import scrapy_splash
from scrapbox_scrapy.items import ScrapboxScrapyItem
//}
scrapy_splashをimportしてます。

上で作成したitems.pyをimportしています。

//emlist[][python]{
LUA_SCRIPT = """
function main(splash)
    splash.private_mode_enabled = false
    splash:go(splash.args.url)
    splash:wait(1)
    html = splash:html()
    splash.private_mode_enabled = true
    return html
end
"""
//}
SplashのLua scriptと呼ばれるJavaScriptを記述します。今回のサイトはプライベートモードだと正しくJavaScriptが動作しないので、Lua scriptを使い一度プライベートモードをオフにしてからJavaScriptレンダリングを行います。

詳しいことは公式ドキュメントとして記載してあります。

Website is not rendered correctly

@<href>{https://splash.readthedocs.io/en/stable/faq.html#website-is-not-rendered-correctly, https://splash.readthedocs.io/en/stable/faq.html#website-is-not-rendered-correctly}

Dockerで最初からプライベートモードをオフにして起動も可能ですが、安全な運用としてはLua scriptにてモード切替をするようにしました。

//emlist[][python]{
name = 'scrapbox_url'
//}
nameはSpiderの名前でクロールするときに指定する名前です。

//emlist[][python]{
allowed_domains = ['scrapbox.io']
//}
allowed_domainsは指定されたドメインのみで動くための設定で、リンクをたどっていくと指定されたドメイン以外にスクレイピングすることを防止します。

//emlist[][python]{
start_urls = ''
//}
start_urlsは空文字にし、コマンドラインのパラメーターで引き渡します。

//emlist[][python]{
def start_requests(self):
    if self.start_url != '':
        yield scrapy_splash.SplashRequest(
            self.start_url,
            self.parse,
            endpoint='execute',
            args={'lua_source': LUA_SCRIPT},
        )
//}

start_requestsを指定しないと、Scrapyはstart_urlに指定されたURLに対してGETメソッドでアクセスします。今回はJavaScriptレンダリングを指定したいので、start_requests用意しSplashRequestでアクセスするようにします。

@<code>{endpoint='execute'}はJavaScriptレンダリングに対してやり取りをするファイルとなり、そこへ@<code>{args={'lua_source': LUA_SCRIPT}}にてLua scriptを実行させます。


//emlist[][python]{
for uls in response.css('ul.page-list'):
    for hrefs in uls.css('a'):
        yield ScrapboxScrapyItem(
            title = hrefs.css('a::text').extract_first(),
            url = response.urljoin(hrefs.css('a::attr(href)')
            .extract_first())
        )
//}



ulタグのclass指定されているpage-listにページ情報があるので、その中にあるaタグの情報を取得します。@<code>{response.css('ul.page-list')}でピックアップしてfor文で回して取得します。


== クローラーの実行
Spiderを作成し各種設定をしたら、クローラーを実行します。 DEBUGのところでurlとtitleがピックアップされていることが確認できます。

なお、今回ログインしないでも見ることができるページを参考例として記載します。

マンガでわかるScrapbox

URL:@<href>{https://scrapbox.io/wakaba-manga/, https://scrapbox.io/wakaba-manga/}
//list[crawl][クローラーの実行][bash]{
scrapy crawl scrapbox_url -a start_url=https://scrapbox.io/wakaba-manga/
//}

== ソースコードについて
この章で使用したソースコードはGitHubにあります。

@<href>{https://github.com/hideaki-kawahara/scrapy-source/tree/chapter4, https://github.com/hideaki-kawahara/scrapy-source/tree/chapter4}

実行する手順を下に記載します。

//emlist[][bash]{
 1. ソースコードをCloneするディレクトリーを作成する。
   @<code>{mkdir -p scrapy-source}
 2. Cloneする。
   @<code>{git clone https://github.com/hideaki-kawahara/scrapy-source.git}
 3. chapter4をcheckoutする。
   @<code>{git checkout chapter4}
 4. 仮想環境を作成する。
   @<code>{python -m venv .venv}
 5. 仮想環境に入る。
   @<code>{source .venv/bin/activate}
 6. ライブラリーをインストールする。
   @<code>{pip install -r requirements.txt}
 7. Dockerディレクトリーに入る。
   @<code>{cd docker}
 8. Dockerを起動する。
   @<code>{docker-compose up -d}
 9. 該当のディレクトーに入る。
   @<code>{cd ../scrapbox_scrapy}
 10. 実行する。
   @<code>{scrapy crawl scrapbox_url}
//}

※実行後に実行キャッシュディレクトリーが作成されるので、他のBrunchをcheckoutしてもchapter4のディレクトリーは消えません。気になるようなら削除してください。

@<code>{rm -rf scrapbox_scrapy}

