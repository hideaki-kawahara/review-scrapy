
= 最初のスクレイピング

ヤフーニュースをスクレイピングしてみます。


== プロジェクトの作成
まずはプロジェクトを作成します。下のコマンドを実行するとyahoo_news_scrapyというディレクトリーが作成されます。

//list[startproject][Projectの作成][bash]{
scrapy startproject yahoo_news_scrapy
//}

ここで作成されるのはScrapyの雛形だけで、Spiderの雛形は作られていません。

次はSpiderを作成します。


//list[genspider][Spiderの作成][bash]{
cd yahoo_news_scrapy
scrapy genspider yahoo_news news.yahoo.co.jp
//}

== アイテム設定
Spiderの雛形が作られたら、下の階層にあるitems.pyを編集します。これはSpiderが出力するアイテムを設定します。

下のようにYahooNewsScrapyItemのところを編集します。

//list[After][編集後のitems.py][python]{
import scrapy


class YahooNewsScrapyItem(scrapy.Item):
    url = scrapy.Field()
    title = scrapy.Field()
//}

== Spider作成
アイテム設定が終わったら、下の階層にあるSpiderである、yahoo_news.pyを編集します。

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

== クローラーの実行
Spiderを作成したら、クローラーを実行します。 DEBUGのところでurlとtitleがピックアップされていることが確認できます。
//list[crawl][クローラーの実行][bash]{
scrapy crawl yahoo_news
//}
