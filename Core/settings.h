#ifndef MSETTINGS_H
#define MSETTINGS_H

#include <QtDebug>
#include <QCoreApplication>
#include <QSettings>
#include <QDir>

class Settings
{
public:
    Settings()
    {
        qInfo() << "initializing settings";
        QCoreApplication::setOrganizationName("Maple-Tree");
        QCoreApplication::setOrganizationDomain("mapleseed.pixxy.in");
        QCoreApplication::setApplicationName("MapleSeed++");
    }
    ~Settings()
    {
        QSettings settings(getfilepath(), QSettings::IniFormat);
        settings.sync();
    }

    static void setValue(const QString &key, const QVariant &value)
    {
        QSettings settings(getfilepath(), QSettings::IniFormat);
        return settings.setValue(key, value);
    }

    static QVariant value(const QString &key)
    {
        QSettings settings(getfilepath(), QSettings::IniFormat);
        return settings.value(key);
    }

    static void clear()
    {
        QSettings settings(getfilepath(), QSettings::IniFormat);
        return settings.clear();
    }

    static QString getdirpath()
    {
        auto homepath(QDir::homePath());
        return QDir(homepath).filePath("MapleSeed");
    }

    static QString getfilepath()
    {
        return QDir(getdirpath()).filePath("MapleSeed.ini");
    }
};

#endif // MSETTINGS_H
