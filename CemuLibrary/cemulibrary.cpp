#include "cemulibrary.h"

CemuLibrary *CemuLibrary::instance;

CemuLibrary::CemuLibrary()
{}

CemuLibrary *CemuLibrary::initialize()
{
    qInfo() << "Cemu Library v1.0.0 loaded";

    if (!instance) {
        instance = new CemuLibrary;
    }

    QString directory(Settings::value("cemu/library").toString());
    instance->init(directory);

    return instance;
}

void CemuLibrary::init(QString directory)
{
    qDebug() << "initializing library";
    if (directory.isEmpty())
    {
        directory = QDir(".").absolutePath();
    }

    library.clear();
    if (!directory.isEmpty()) {
        if (QDir(directory).exists()) {
            directory = QDir(directory).absolutePath();
        }
        else {
            qDebug() << "invalid directory" << directory;
            return;
        }
    }

    QStringList list(QDir(directory).entryList());
    QtConcurrent::blockingMapped(list, &CemuLibrary::processItem);
}

TitleInfo *CemuLibrary::find(QString id)
{
    if (instance->library.contains(id.toUpper()))
    {
        return instance->library[id.toUpper()];
    }
    return nullptr;
}

QString CemuLibrary::XmlValue(const QFileInfo &metaxml, const QString &field)
{
    QString value;
    if (QFile(metaxml.filePath()).exists()) {
        QDomDocument doc;
        QFile file(metaxml.filePath());
        if (!file.open(QIODevice::ReadOnly) || !doc.setContent(&file))
            return nullptr;

        QDomNodeList rates = doc.elementsByTagName("menu");
        for (int i = 0; i < rates.size(); i++) {
            QDomNode n = rates.item(i);
            QDomElement element = n.firstChildElement(field);
            if (element.isNull())
                continue;
            value = element.text();
        }
    }
    return value.toUpper();
}

QVariant CemuLibrary::processItem(const QString &d)
{
    auto self = CemuLibrary::instance;
    if (!self) {
        return NULL;
    }
    QString directory(Settings::value("cemu/library").toString());

    QDir baseDir(QDir(directory).filePath(d));
    QString metaFile(baseDir.filePath("meta/meta.xml"));
    if (QFileInfo(metaFile).exists())
    {
        emit self->OnNewEntry(metaFile);
    }
    return d;
}
