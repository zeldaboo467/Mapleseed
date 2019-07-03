#ifndef TITLEINFO_H
#define TITLEINFO_H

#include <QString>
#include <QDir>
#include <QDirIterator>
#include "settings.h"

enum TitleType { Game = 0, Demo = 1, Patch = 2, Dlc = 3, None };

struct TitleInfo
{
    TitleInfo() {}
    TitleInfo(QMap<QString, QVariant> qdata) : data(qdata) {}
    TitleInfo& operator=(const QMap<QString, QVariant>& other)
    {
        data = other;
        return *this;
    }
    QString id() { return data["id"].toString().toUpper(); }
    QString key() { return data["key"].toString().toUpper(); }
    QString name() { return data["name"].toString().replace('\n', ' '); }
    QString region() { return data["region"].toString().toUpper(); }
    QString productcode() { return data["productcode"].toString(); }
    QString dir()
    {
        QDir dir(Settings::value("cemu/library").toString());

        QFileInfo fileInfo(XmlPath);
        if (fileInfo.exists())
        {
            dir = QDir(fileInfo.dir());
            dir.cdUp();
            return dir.absolutePath();
        }

        switch (titleType())
        {
        case TitleType::Patch:
            dir = dir.filePath("Patch");
            break;

        case TitleType::Dlc:
            dir = dir.filePath("DLC");
            break;

        case TitleType::Demo:
            dir = dir.filePath("Demo");
            break;

        case TitleType::Game:
        case TitleType::None:
            break;
        }

        QString name(formatName());
        name.remove(QRegExp("[\\/:*?""<>|]"));
        return QDir(dir.filePath(name)).absolutePath();
    }
    QString coverArt()
    {
        QString code(getProductCode());
        QString cover;

        QDir directory(Settings::value("cemu/coversDir").toString());
        QStringList nameFilter("*" + code + "*.jpg");
        QStringList list = directory.entryList(nameFilter);
        if (list.count() > 0){
            cover = directory.filePath(list.first());
        }
        else {
            cover = directory.filePath("!.jpg");
        }

        return cover;
    }
    QString getProductCode()
    {
        if (!productcode().isEmpty()) {
            return productcode().right(4);
        }
        return nullptr;
    }
    QString formatName() {
        switch (titleType()) {
        case TitleType::Patch:
            return QString("[") + region().toUpper() + QString("][Update] ") + name();
        case TitleType::Dlc:
            return QString("[") + region().toUpper() + QString("][DLC] ") + name();
        case TitleType::Demo:
            return QString("[") + region().toUpper() + QString("][Demo] ") + name();
        case TitleType::Game:
            return QString("[") + region().toUpper() + QString("] ") + name();
        case TitleType::None:
            return QString("[") + region().toUpper() + QString("][Unknown] ") + name();
        }
        return nullptr;
    }
    TitleType titleType()
    {
        if (id().size() == 16)
        {
            QChar ch = id().at(7);
            if (ch == 'e' || ch == 'E') {
                return TitleType::Patch;
            }
            else if (ch == 'c' || ch == 'C') {
                return TitleType::Dlc;
            }
            else if (ch == '2') {
                return TitleType::Demo;
            }
            else if (ch == '0') {
                return TitleType::Game;
            }
        }
        return None;
    }
    QString Rpx()
    {
        QString root = QFileInfo(XmlPath).dir().filePath("../code");
        QDirIterator it(root, QStringList() << "*.rpx", QDir::NoFilter);
        while (it.hasNext())
        {
            return QFileInfo(it.next()).absoluteFilePath();
        }
        return nullptr;
    }

    QMap<QString, QVariant> data;
    QString XmlPath;
};

#endif // TITLEINFO_H
