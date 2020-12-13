
= 最初のスクレイピング

ヤフーをスクレイピングしてみます。


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

Spiderの雛形が作られたら、階層を下がってSpiderが出力するアイテムを設定するitems.pyを編集します。
//list[items][階層を下がります][bash]{
cd yahoo_news_scrapy
//}

下のようにYahooNewsScrapyItemのところを編集します。

//list[After][編集後のitems.py][python]{
import scrapy


class YahooNewsScrapyItem(scrapy.Item):
    url = scrapy.Field()
    title = scrapy.Field()
//}

アイテムの設定が終わったら、さらに階層を下がって、Spiderを作成します。

yahoo_news.pyを編集します。
//list[yahoo_news][階層下がります][bash]{
cd yahoo_news_scrapy
//}

編集内容は下のとおりになります。

1. 上で作成したitems.pyをimportします。
2. start_urlsを取得したいURLに変更します。
@<href>{https://news.yahoo.co.jp/topics/it, https://news.yahoo.co.jp/topics/it}に変更します。
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
                url=items.css('a.newsFeed_item_link::attr(href)').extract_first(),
                title=items.css('div.newsFeed_item_title::text').extract_first()
            )
//}

Spiderを作成したら、クローラーを実行します。 DEBUGのところでurlとtitleがピックアップされていることが確認できます。
//list[crawl][クローラーの実行][bash]{
scrapy crawl yahoo_news
//}



