
= PipelineとORM

Qiitaのトレンドをスクレイピングして分析してみます。

分析することで言語やフレームワークなどのトレンドが日によって変化することが見えてきます。

== ORMの導入
ローカル環境で動かしますが、ORM（オブジェクト関係マッピング、Object-Relational Mapping）を使用してデータベースにアクセスします。

今回はORMとしてSqlalchemyを利用します。環境構築@<chap>{building-environmen}をしたあとにSqlalchemyをインストールします。
//list[sqlalchemy][sqlalchemyの用意][bash]{
pip install sqlalchemy
pip install psycopg2
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
      TZ: Asia/Tokyo
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
    id SERIAL,
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
Spiderの雛形が作られたらitems.pyを編集します。

ファイルがある場所は@<code>{scrapy-source/qiita_trend_scrapy/qiita_trend_scrapy/items.py}です。

こちらはSpiderが出力するアイテムを設定するところになります。下のようにQiitaTrendScrapyItemのところを編集します。

//list[item][編集後のitems.py][python]{
import scrapy


class QiitaTrendScrapyItem(scrapy.Item):
    keyword = scrapy.Field()
    count = scrapy.Field()
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

次はpipelineの設定をします。


== Pipelineの設定
データベースに記録するにはPipelineの設定を行います。キャッシュを設定したにsettings.pyで行います。

ファイルがある場所は@<code>{scrapy-source/qiita_trend_scrapy/qiita_trend_scrapy/settings.py}です。

下のようにITEM_PIPELINESのところからコメントアウトされているので、コメントアウトを解除するように編集します。

//list[ITEM_PIPELINES][pipelineの設定][python]{
ITEM_PIPELINES = {
   'qiita_trend_scrapy.pipelines.QiitaTrendScrapyPipeline': 300,
}
//}



次はデータベースの設定ファイルを作成します。


== データベースの設定
データベースの設定ファイルを作成します。

ファイルがある場所は@<code>{scrapy-source/qiita_trend_scrapy/qiita_trend_scrapy/database.py}です。

もしデータベースのデバック情報が不要なときは@<code>{echo=False}にします。

//list[DATABASE][データベースの設定][python]{
from sqlalchemy import create_engine
from sqlalchemy.orm import scoped_session, sessionmaker
from sqlalchemy.ext.declarative import declarative_base

DATABASE = 'postgresql://%s:%s@%s/%s' % (
    "postgres",
    "",
    "127.0.0.1",
    "postgres",
)

ENGINE = create_engine(
    DATABASE,
    encoding = "utf-8",
    echo=True
)

session = scoped_session(
    sessionmaker(
        autocommit = False,
        autoflush = False,
        bind = ENGINE
    )
)

Base = declarative_base()
Base.query = session.query_property()
//}


次はテーブル情報を作成します。


== テーブル情報の作成
テーブル情報ファイルを作成します。

ファイルがある場所は@<code>{scrapy-source/qiita_trend_scrapy/qiita_trend_scrapy/tag.py}です。

//list[tag][テーブル情報ファイルの作成][python]{
import sys
from sqlalchemy import Column, Integer, Text, DateTime, Sequence
from sqlalchemy.sql.functions import current_timestamp
from . database import ENGINE, Base


class Tag(Base):
    __tablename__ = 'tags'
    id = Column('id', Integer, Sequence('tags_id_seq'), primary_key=True)
    keyword = Column('keyword', Text)
    count = Column('count', Integer)
    created_at = Column(DateTime(timezone=True), nullable=False, server_default=current_timestamp())
    updated_at = Column(DateTime(timezone=True), nullable=False, server_default=current_timestamp())

def main(args):
    Base.metadata.create_all(bind=ENGINE)


if __name__ == '__main__':
    main(sys.argv)
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


//list[QiitaTrendSpider][Spiderの編集][python]{
import scrapy
from qiita_trend_scrapy.items import QiitaTrendScrapyItem


class QiitaTrendSpider(scrapy.Spider):
    name = 'qiita_trend'
    allowed_domains = ['qiita.com']
    start_urls = ['http://qiita.com/']

    def parse(self, response):
        keywords = {}
        for items in response.css('a.css-4czcte'):
            key = items.css('a.css-4czcte::text').extract_first()
            if (key in keywords.keys()):
                keywords[key] = keywords[key] + 1
            else:
                keywords[key] = 1

        for keyword, count in keywords.items():
            yield QiitaTrendScrapyItem(keyword = keyword, count = count)
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

