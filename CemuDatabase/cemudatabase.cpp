#include "cemudatabase.h"

CemuDatabase *CemuDatabase::instance = new CemuDatabase;

CemuDatabase::CemuDatabase() = default;

CemuDatabase* CemuDatabase::initialize()
{
    qInfo() << "Database library v1.0.0 loaded";

    if (!instance)
    {
        instance = new CemuDatabase;
    }

    QString jsonpath(Settings::value("cemu/database").toString());
    instance->init(jsonpath);

    return instance;
}

void CemuDatabase::init(QString jsonpath)
{
    qDebug() << "initializing database";

    if (jsonpath.isEmpty() || !QFileInfo(jsonpath).exists())
    {
        jsonpath = QDir(QDir(".").absolutePath()).filePath("titlekeys.json");
        Settings::setValue("cemu/database", jsonpath);
    }

    if (!QFileInfo(jsonpath).exists())
    {
        DownloadFile(QUrl("http://pixxy.in/mapleseed/titlekeys.json"), jsonpath);
    }

    QFile qfile(jsonpath);
    if (!qfile.exists() || !qfile.open(QIODevice::ReadOnly))
    {
        qCritical() << qfile.errorString();
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(qfile.readAll());
    if (doc["titlekeys"].isArray())
    {
        QJsonArray array = doc["titlekeys"].toArray();
        for(auto item : array.toVariantList())
        {
            ParseJsonItem(item);
        }
        qDebug() << "initialized" << database.count() << "database entries";
        emit OnLoadComplete();
    }
}

TitleInfo *CemuDatabase::Create(QString xmlpath)
{
    QString id = XmlValue(xmlpath, "title_id");
    TitleInfo *info = find(id);
    if (info)
    {
        info->XmlPath = xmlpath;
        return info;
    }
    return nullptr;
}

QString CemuDatabase::XmlValue(const QFileInfo &metaxml, const QString &field)
{
    QString value;
    if (QFile(metaxml.filePath()).exists())
    {
        QDomDocument doc;
        QFile file(metaxml.filePath());
        if (!file.open(QIODevice::ReadOnly) || !doc.setContent(&file))
            return nullptr;

        QDomNodeList rates = doc.elementsByTagName("menu");
        for (int i = 0; i < rates.size(); i++)
        {
            QDomNode n = rates.item(i);
            QDomElement element = n.firstChildElement(field);
            if (element.isNull())
                continue;
            value = element.text();
        }
    }
    return value;
}

TitleInfo *CemuDatabase::find(QString id)
{
    if (instance->database.contains(id.toUpper()))
    {
        return &instance->database[id.toUpper()];
    }
    return nullptr;
}

bool CemuDatabase::ValidId(QString id)
{
    auto& db = CemuDatabase::instance->database;
    if (db.contains(id.toUpper()))
    {
        return true;
    }
    return false;
}

void CemuDatabase::ParseJsonItem(QVariant &item)
{
    QString id(item.toMap()["id"].toString());
    QString key(item.toMap()["key"].toString());
    if (!id.isEmpty() && !key.isEmpty())
    {
        database[id] = item.toMap();
        emit OnNewEntry(&database[id]);
    }
}

char *CemuDatabase::DownloadTMD(QString id, QString ver, QString dir)
{
    QString tmdpath(dir + "/tmd");
    QString tmdurl("http://ccs.cdn.wup.shop.nintendo.net/ccs/download/" + id + "/tmd");
    if (!ver.isEmpty()){
        tmdurl += "." + ver;
    }

    QByteArray buffer;
    if (!QFile(tmdpath).exists())
    {
        DownloadFile(tmdurl, tmdpath);
    }

    QFile* tmdfile = new QFile(tmdpath);
    if (!tmdfile->open(QIODevice::ReadWrite))
    {
        qCritical() << tmdfile->errorString();
        return nullptr;
    }

    char* data = new char[static_cast<qulonglong>(tmdfile->size())];
    tmdfile->read(data, tmdfile->size());
    tmdfile->close();

    return data;
}

void CemuDatabase::DownloadFile(QUrl url, QString path)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly))
    {
        qCritical() << file.errorString();
        return;
    }

    QNetworkAccessManager manager;
    QNetworkRequest request(url);

    QEventLoop loop;
    QNetworkReply *reply = manager.get(request);
    connect(reply, &QNetworkReply::readyRead, [&]{ file.write(reply->readAll()); });
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(reply, &QNetworkReply::finished, [&]
    {
        if (isHttpRedirect(reply))
        {
            file.close();
            auto qVariantUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
            DownloadFile(qVariantUrl.toUrl(), path);
        }
    });
    loop.exec();

    delete reply;
}

bool CemuDatabase::isHttpRedirect(QNetworkReply *reply)
{
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    return statusCode == 301 || statusCode == 302 || statusCode == 303 || statusCode == 305 || statusCode == 307 || statusCode == 308;
}

QByteArray CemuDatabase::CreateTicket(QString id, QString key, QString ver, QString dir)
{
    if (id.isEmpty() || key.isEmpty())
    {
        qWarning() << "invalid id or key" << id << key;
    }
    QByteArray data;
    id = id.toUpper();
    key = key.toUpper();

    data.insert(0x1E6, QByteArray::fromHex(ver.toLatin1()));
    data.insert(0x1BF, QByteArray::fromHex(key.toLatin1()));
    data.insert(0x2CC, QByteArray::fromHex(id.toLatin1()));

    QFile file(dir + "/cetk");
    if (!file.open(QIODevice::WriteOnly))
    {
        qCritical() << file.errorString();
        return nullptr;
    }
    file.write(data);
    file.close();

    return data;
}
