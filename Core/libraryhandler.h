#ifndef LIBRARYHANDLER_H
#define LIBRARYHANDLER_H

#include <QObject>
#include <QLibrary>
#include "settings.h"

class LibraryHandler : public QObject
{
    Q_OBJECT
public:
    explicit LibraryHandler(QObject *parent = nullptr);

    QLibrary *LoadPlugin(QString name, QString library);

    QString DebugDir(QString name);

    QLibrary *CemuDb();

    CemuDatabase *newCemuDatabase();

private:
    QString librariesDir;
    QMap<QString, QLibrary*> libraries;

signals:

public slots:
};

#endif // LIBRARYHANDLER_H
