= データベースを使用する

Qiitaのトレンドをスクレイピングして分析してみます。

データベースに情報を格納して分析することで言語やフレームワークなどのトレンドが日によって変化することが見えてきます。なお、今回はデータベースに情報を格納するところまでとします。

対象サイトは「Qiita」のトップページトレンドページです。（タグページはログインしないと見ることができません。）

URL:@<href>{https://qiita.com/, https://qiita.com/}@<br>{}

5段階評価で難易度を記載します。「Qiita」サイトの難易度は1つですが、データベースを使用するので難易度は星2つです。

難易度：★★

== ORMの導入
ローカル環境で動かしますが、ORM（オブジェクト関係マッピング、Object-Relational Mapping）を使用してデータベースにアクセスします。

今回はORMとしてSqlalchemyを利用します。環境構築@<chap>{building-environmen}をしたあとにSqlalchemyとPostgreSQLを使用するのでpsycopg2をインストールします。
//list[sqlalchemy][sqlalchemyとpsycopg2の用意][bash]{
pip install sqlalchemy
pip install psycopg2
//}


== データベースの準備
次にデータベースを用意します。ローカル環境にデータベースを入れるか、Dockerでデータベースを入れます。

今回はDockerでデータベースを入れてみます。Dockerの入れ方は、この書籍では記載しませんが、dockerとdocker-composeを事前にインストールしておきます。

Docker用のディレクトリーを作成します。
//list[docker][Docker用ディレクトリー作成][bash]{
mkdir -p docker
cd docker
//}

Docker用ディレクトリーにdocker-compose.ymlファイルを作成します。今回はPostgreSQLを使います。

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

データベースの初期設定ファイルを作成します。

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
次にプロジェクトを作成します。Dockerディレクトリーから上の階層に戻り、下のコマンドを実行するとqiita_trend_scrapyというディレクトリーが作成されます。

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
Spiderの雛形が作られたらitems.pyを編集します。編集するファイルは@<code>{scrapy-source/qiita_trend_scrapy/qiita_trend_scrapy/items.py}です。


こちらはSpiderが出力するアイテムを設定するところになります。下のようにQiitaTrendScrapyItemのところを編集します。

//list[item][編集後のitems.py][python]{
import scrapy


class QiitaTrendScrapyItem(scrapy.Item):
    keyword = scrapy.Field()
    count = scrapy.Field()
//}

次にディレイタイムの設定とキャッシュの設定をしますが、@<hd>{first-step|ディレイタイムの設定}と@<hd>{first-step|キャッシュの設定}と同じになるので、同じように設定しておきます。

次はpipelineの設定をします。


== Pipelineの設定
データベースに記録するにはPipelineの設定を行います。settings.pyで行います。編集するファイルは@<code>{scrapy-source/qiita_trend_scrapy/qiita_trend_scrapy/items.py}です。

下のようにITEM_PIPELINESのところからコメントアウトされているので、コメントアウトを解除するように編集します。

//list[ITEM_PIPELINES][pipelineの設定][python]{
ITEM_PIPELINES = {
   'qiita_trend_scrapy.pipelines.QiitaTrendScrapyPipeline': 300,
}
//}

次はデータベースの設定ファイルを作成します。

== データベースの設定
データベースの設定ファイルを作成します。作成するファイルは@<code>{scrapy-source/qiita_trend_scrapy/qiita_trend_scrapy/database.py}です。

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

もしデータベースのデバック情報が不要なときは@<code>{echo=False}にします。

次はテーブル情報を作成します。


== テーブル情報の作成
テーブル情報ファイルを作成します。作成するファイルは@<code>{scrapy-source/qiita_trend_scrapy/qiita_trend_scrapy/tag.py}です。

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
    created_at = Column(DateTime(timezone=True), nullable=False,@<embed>$|latex|\linebreak\hspace*{25ex}$ server_default=current_timestamp())
    updated_at = Column(DateTime(timezone=True), nullable=False,@<embed>$|latex|\linebreak\hspace*{25ex}$ server_default=current_timestamp())

def main(args):
    Base.metadata.create_all(bind=ENGINE)


if __name__ == '__main__':
    main(sys.argv)
//}

次はPipelineを作成します。

== Pipeline作成
Pipelineでは受け取ったitemに対して処理をします。作成するファイルは@<code>{scrapy-source/qiita_trend_scrapy/qiita_trend_scrapy/pipeline.py}です。


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

Tagテーブル情報のインスタンスを用意し、@<code>{session.add}でレコードを追加して@<code>{session.commit}すると、データベースに記録します。

次はSpiderを作成します。

== Spider作成
Spiderであるqiita_trend.pyを編集します。編集するファイルは@<code>{scrapy-source/qiita_trend_scrapy/qiita_trend_scrapy/spiders/qiita_trend.py}です。

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

=== 解説
Spiderのソースコードを解説します。

他の章と同じようにスクレイピングします。

//emlist[][python]{
from qiita_trend_scrapy.items import QiitaTrendScrapyItem
//}
上で作成したitems.pyをimportしています。

//emlist[][python]{
name = 'qiita_trend'
//}
nameはSpiderの名前でクロールするときに指定する名前です。

//emlist[][python]{
allowed_domains = ['qiita.com']
//}
allowed_domainsは指定されたドメインのみで動くための設定で、リンクをたどっていくと指定されたドメイン以外にスクレイピングすることを防止します。

//emlist[][python]{
start_urls = ['http://qiita.com/']
//}
start_urlsはスクレイピングするURLを配列で指定します。デフォルトではstart_urlsで指定されたURLをスクレイピングしてparse関数に引き渡します。

//emlist[][python]{
keywords = {}
for items in response.css('a.css-4czcte'):
    key = items.css('a.css-4czcte::text').extract_first()
    if (key in keywords.keys()):
        keywords[key] = keywords[key] + 1
    else:
        keywords[key] = 1
//}

今回の例ではQiitaタグは、class指定されているcss-4czcteのaタグをすべて取得します。@<code>{response.css('a.css-4czcte')}でピックアップしてfor文で回します。すでに存在しているQiitaタグがあったら辞書の数値をインクリメントし、存在しないときは1で初期化します。

//emlist[][python]{
for keyword, count in keywords.items():
    yield QiitaTrendScrapyItem(keyword = keyword, count = count)
//}

カウントが終わったらデータを引き引き渡します。

== クローラーの実行
Spiderを作成し各種設定をしたら、Dockerを起動しクローラーを実行します。 DEBUGメッセージにSqlalchemyの実行結果が表示されていることが確認できます。
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
cd ../docker
docker-compose down
//}

== ソースコードについて
この章で使用したソースコードはGitHubにあります。

@<href>{https://github.com/hideaki-kawahara/scrapy-source/tree/chapter3, https://github.com/hideaki-kawahara/scrapy-source/tree/chapter3}

実行する手順を下に記載します。

//emlist[][bash]{
 1. ソースコードをCloneするディレクトリーを作成する。
   @<code>{mkdir -p scrapy-source}
 2. Cloneする。
   @<code>{git clone https://github.com/hideaki-kawahara/scrapy-source.git}
 3. chapter3をcheckoutする。
   @<code>{git checkout chapter3}
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
 9. 該当のディレクトリーに入る。
   @<code>{cd ../qiita_trend_scrapy}
 10. 実行する。
   @<code>{scrapy crawl qiita_trend}
//}

※実行後に実行キャッシュディレクトリーが作成されるので、他のBrunchをcheckoutしてもchapter3のディレクトリーは消えません。気になるようなら削除してください。

@<code>{rm -rf qiita_trend_scrapy}

