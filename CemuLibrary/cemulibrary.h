#ifndef CEMULIBRARY_H
#define CEMULIBRARY_H

#include "cemulibrary_global.h"
class CEMULIBRARYSHARED_EXPORT CemuLibrary : public QObject
{
    Q_OBJECT
public:
    CemuLibrary();

    static CemuLibrary *initialize();

    void init(QString directory);

    static TitleInfo *find(QString id);

    static QString XmlValue(const QFileInfo &metaxml, const QString &field);

    static QVariant processItem(const QString &d);

    static CemuLibrary *instance;

    QMap<QString, TitleInfo*> library;

signals:
    void OnNewEntry(QString xmlfile);
};

#endif // CEMULIBRARY_H
