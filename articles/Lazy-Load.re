= Lazy loading画面のスクレイピング

Lazy loading画面をスクレイピングします。

Lazy loading画面はJavaScriptが画面のレンダリングをして、ブラウザで表示されていないところは表示を行わず、利用者がスクロールを行って新たに画面が必要になったら画面のレンダリングをして画面表示する仕組みです。

自動的に次ページをめくるサイトに多く見られる手法です。

前の章に引き続きJavaScriptレンダリングとしてSplashを利用します。

対象サイトは「技術書典10オンラインマーケット」でログイン無しにてアクセスできるページです。

URL:@<href>{https://techbookfest.org/event/tbf10/market/newbook, https://techbookfest.org/event/tbf10/market/newbook}@<br>{}

5段階評価で難易度を記載します。「技術書典10オンラインマーケット」サイトの難易度は星5つです。

難易度：★★★★★


== 前準備
JavaScriptレンダリングをDockerで準備します。


@<chap>{building-environmen}をしたあとに@<hd>{javascript|JavaScriptレンダリングの導入}と@<hd>{javascript|Splashの準備}を行います。

注意：Dockerの使用メモリはデフォルトでは2Mバイトで、JavaScriptレンダリングとして動いているDockerコンテナーがメモリーエラーで落ちることがあります。8Mバイトに増やしてください。

== プロジェクトの作成
次にプロジェクトを作成します。Dockerディレクトリーに居る場合は上の階層に戻り、下のコマンドを実行するとtechbookfest_scrapyというディレクトリーが作成されます。

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
Spiderの雛形が作られたらitems.pyを編集します。編集するファイルは@<code>{techbookfest_scrapy/techbookfest_scrapy/items.py}です。

こちらはSpiderが出力するアイテムを設定するところになります。下のようにTechbookfestScrapyItemのところを編集します。

//list[item][編集後のitems.py][python]{
import scrapy


class TechbookfestScrapyItem(scrapy.Item):
    title = scrapy.Field()
    url = scrapy.Field()
    money1 = scrapy.Field()
    money2 = scrapy.Field()
    money3 = scrapy.Field()
    money4 = scrapy.Field()
    openning = scrapy.Field()
//}

次にディレイタイムの設定とキャッシュの設定をしますが、@<hd>{first-step|ディレイタイムの設定}と@<hd>{first-step|キャッシュの設定}と同じになるので、同じように設定しておきます。

あわせて@<hd>{javascript|JavaScriptレンダリング用の設定}も設定しておきます。


== Spider作成
Spiderであるtechbookfest_url.pyを編集します。編集するファイルは@<code>{techbookfest_scrapy/techbookfest_scrapy/spiders/techbookfest_url.py}です。

できたものは下のようになります。

//list[QiitaTrendSpider][Spiderの編集][python]{
import scrapy
import scrapy_splash
from techbookfest_scrapy.items import TechbookfestScrapyItem


LUA_SCRIPT_LIST = """
function main(splash, args)
  local scroll_to = splash:jsfunc("window.scrollTo")
  splash:go(args.url)
  for _ = 1, 20 do
    scroll_to(0, 10000)
    splash:wait(1)
  end
  html = splash:html()
  return html
end
"""

LUA_SCRIPT_DETAIL = """
function main(splash)
    splash.private_mode_enabled = false
    splash:go(splash.args.url)
    splash:wait(5)
    html = splash:html()
    splash.private_mode_enabled = true
    return html
end
"""

USER_AGENT = 'Mozilla/5.0 (Macintosh; Intel Mac OS X 11_1_0)@<embed>$|latex|\linebreak\hspace*{25ex}$ AppleWebKit/537.36 (KHTML, like Gecko) Chrome/87.0.4280.88 Safari/537.36'

class TechbookfestUrlSpider(scrapy.Spider):
    name = 'techbookfest_url'
    allowed_domains = ['techbookfest.org']
    start_url = 'https://techbookfest.org/event/tbf10/market/newbook'

    def start_requests(self):
        yield scrapy_splash.SplashRequest(
            self.start_url,
            self.parse,
            headers={'User-Agent': USER_AGENT},
            endpoint='execute',
            args={'lua_source': LUA_SCRIPT_LIST},
        )

    def parse(self, response):
        for hrefs in response.css('div.r-bnwqim'):
            href = hrefs.css('a::attr(href)').extract_first()
            if href is not None:
                yield scrapy_splash.SplashRequest(
                    response.urljoin(href),
                    self.after_parse,
                    headers={'User-Agent': USER_AGENT},
                    endpoint='execute',
                    args={'lua_source': LUA_SCRIPT_DETAIL},
                )

    def after_parse(self, response):
        money_items = [''] * 4
        openning = ''

        for information in response.css('div.css-1dbjc4n div.r-18u37iz'):
            for h2_tag in information.css('div.css-1dbjc4n div.r-13awgt0@<embed>$|latex|\linebreak\hspace*{25ex}$ div.r-1jkjb'):
                title = h2_tag.css('h2::text').extract_first()
            for money_tag in information.css('div.css-18t94o4'):
                money = money_tag.css('div.css-901oao::text').extract()
                if len(money) == 0:
                    pass
                elif '電子版' in money[0]:
                    money_items[0] = ':'.join(money)
                elif '電子＋' in money[0]:
                    money_items[1] = ':'.join(money)
                elif '梅' in money[0]:
                    money_items[1] = ':'.join(money)
                elif '竹' in money[0]:
                    money_items[2] = ':'.join(money)
                elif '松' in money[0]:
                    money_items[3] = ':'.join(money)
            for event in information.css('div.css-1dbjc4n'):
                openning_tag = event.css('div.r-1enofrn::text')@<embed>$|latex|\linebreak\hspace*{25ex}$.extract_first()
                if openning_tag is None:
                    pass
                elif '初出イベント' in openning_tag:
                    openning = openning_tag

        yield TechbookfestScrapyItem(
            title = title,
            money1 = money_items[0],
            money2 = money_items[1],
            money3 = money_items[2],
            money4 = money_items[3],
            openning = openning,
            url=response.url)
//}

=== 解説
Spiderのソースコードを解説します。

他の章と同じようにスクレイピングします。

//emlist[][python]{
import scrapy_splash
from techbookfest_scrapy.items import TechbookfestScrapyItem
//}
scrapy_splashをimportしてます。

上で作成したitems.pyをimportしています。

//emlist[][python]{
LUA_SCRIPT_LIST = """
function main(splash, args)
  local scroll_to = splash:jsfunc("window.scrollTo")
  splash:go(args.url)
  for _ = 1, 20 do
    scroll_to(0, 10000)
    splash:wait(1)
  end
  html = splash:html()
  return html
end
"""
//}
SplashのLua scriptと呼ばれるJavaScriptを記述します。書籍一覧はLazy loadingを使用しているので、それに対応するためSplashを使用して表示したあとJavaScriptで少しずつスクロールさせて、そのあとHTMLを取得しています。

この辺りの数値を大きくすると504タイムアウトを起こすのでいじらないでください。

//emlist[][python]{
LUA_SCRIPT_DETAIL = """
function main(splash)
    splash.private_mode_enabled = false
    splash:go(splash.args.url)
    splash:wait(5)
    html = splash:html()
    splash.private_mode_enabled = true
    return html
end
"""
//}
書籍詳細もLazy loadingを使用しているので、Waitを入れています。前の章と同じようにプライベートモードだと正しくJavaScriptが動作しないので、プライベートモードをオフにしてからJavaScriptレンダリングを行います。

//emlist[][python]{
USER_AGENT = 'Mozilla/5.0 (Macintosh; Intel Mac OS X 11_1_0)@<embed>$|latex|\linebreak\hspace*{25ex}$ AppleWebKit/537.36 (KHTML, like Gecko) Chrome/87.0.4280.88 Safari/537.36'
//}

Scrapyの設定でユーザーエージェントが利用できますが、JavaScriptレンダリングでは未設定になるためSpiderの方で用意します。

//emlist[][python]{
name = 'techbookfest_url'
//}
nameはSpiderの名前でクロールするときに指定する名前です。


//emlist[][python]{
allowed_domains = ['techbookfest.org']
//}
allowed_domainsは指定されたドメインのみで動くための設定で、リンクをたどっていくと指定されたドメイン以外にスクレイピングすることを防止します。

//emlist[][python]{
start_url = 'https://techbookfest.org/event/tbf10/market/newbook'
//}
start_urlsはスクレイピングするURLを配列で指定します。今回はLua scriptが使用します。

//emlist[][python]{
def start_requests(self):
    yield scrapy_splash.SplashRequest(
        self.start_url,
        self.parse,
        headers={'User-Agent': USER_AGENT},
        endpoint='execute',
        args={'lua_source': LUA_SCRIPT_LIST},
    )
//}

start_requestsを指定しないと、Scrapyはstart_urlに指定されたURLに対してGETメソッドでアクセスします。今回はJavaScriptレンダリングを指定したいので、start_requests用意しSplashRequestでアクセスするようにします。

@<code>{endpoint='execute'}はJavaScriptレンダリングに対してやり取りをするファイルとなり、そこへ@<code>{args={'lua_source': LUA_SCRIPT_LIST}}にてLua scriptを実行させ書籍一覧の取得します。

@<code>{headers={'User-Agent': USER_AGENT}}でユーザーエージェントを指定します。


//emlist[][python]{
for hrefs in response.css('div.r-bnwqim'):
    href = hrefs.css('a::attr(href)').extract_first()
    if href is not None:
        yield scrapy_splash.SplashRequest(
            response.urljoin(href),
            self.after_parse,
            headers={'User-Agent': USER_AGENT},
            endpoint='execute',
            args={'lua_source': LUA_SCRIPT_DETAIL},
        )
//}

divタグのclass指定されているr-bnwqimに書籍詳細の情報があるので、その中にあるaタグの情報を取得します。@<code>{response.css('div.r-bnwqim')}でピックアップしてfor文で回して書籍詳細のLua scriptを実行させます。

@<code>{args={'lua_source': LUA_SCRIPT_DETAIL}}にてLua scriptを実行させ書籍一覧の取得します。

//emlist[][python]{
for information in response.css('div.css-1dbjc4n div.r-18u37iz'):
    for h2_tag in information.css('div.css-1dbjc4n div.r-13awgt0 div.r-1jkjb'):
        title = h2_tag.css('h2::text').extract_first()
    for money_tag in information.css('div.css-18t94o4'):
        money = money_tag.css('div.css-901oao::text').extract()
        if len(money) == 0:
            pass
        elif '電子版' in money[0]:
            money_items[0] = ':'.join(money)
        elif '電子＋' in money[0]:
            money_items[1] = ':'.join(money)
        elif '梅' in money[0]:
            money_items[1] = ':'.join(money)
        elif '竹' in money[0]:
            money_items[2] = ':'.join(money)
        elif '松' in money[0]:
            money_items[3] = ':'.join(money)
    for event in information.css('div.css-1dbjc4n'):
        openning_tag = event.css('div.r-1enofrn::text').extract_first()
        if openning_tag is None:
            pass
        elif '初出イベント' in openning_tag:
            openning = openning_tag
//}

divタグのclassを検索して各種情報があるので、その中にあるdivタグのテキスト情報を取得します。似たclassが多数あるので、複数することで余計な情報を取得しないようにしてます。

@<code>{h2_tag.css('h2::text')}で書籍のタイトルを取得してます。

@<code>{money_tag.css('div.css-901oao::text')}で価格を取得しますが、複数あるので内容を確認して別の配列に入れておきます。

@<code>{event.css('div.r-1enofrn::text')}で初出イベントを取得します。この項目は出展サークルの任意入力なので「技術書典10」の数字が全角のときや半角のときがありますが、そのままとします。

//emlist[][python]{
yield TechbookfestScrapyItem(
    title = title,
    money1 = money_items[0],
    money2 = money_items[1],
    money3 = money_items[2],
    money4 = money_items[3],
    openning = openning,
    url=response.url)
//}
書籍の詳細を引き渡します。

@<code>{response.url}はスクレイピングしているURLなので、これもあわせて引き渡します。

== クローラーの実行
Spiderを作成し各種設定をしたら、クローラーを実行します。 DEBUGのところで情報（title,money1,money2,money3,money4,openning,url）がピックアップされていることが確認できます。

//list[crawl][クローラーの実行][bash]{
scrapy crawl techbookfest_url
//}

※非常にゆっくりスクレイピングするので時間のあるときに行ってください。

== ソースコードについて
この章で使用したソースコードはGitHubにあります。

@<href>{https://github.com/hideaki-kawahara/scrapy-source/tree/chapter5, https://github.com/hideaki-kawahara/scrapy-source/tree/chapter5}

実行する手順を下に記載します。

//emlist[][bash]{
 1. ソースコードをCloneするディレクトリーを作成する。
   @<code>{mkdir -p scrapy-source}
 2. Cloneする。
   @<code>{git clone https://github.com/hideaki-kawahara/scrapy-source.git}
 3. chapter5をcheckoutする。
   @<code>{git checkout chapter5}
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
   @<code>{cd ../techbookfest_scrapy}
 10. 実行する。
   @<code>{scrapy crawl techbookfest_url}
//}

※実行後に実行キャッシュディレクトリーが作成されるので、他のBrunchをcheckoutしてもchapter5のディレクトリーは消えません。気になるようなら削除してください。

@<code>{rm -rf techbookfest_scrapy}

