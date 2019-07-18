#ifndef CEMUDB_GLOBAL_H
#define CEMUDB_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QObject>
#include <QtDebug>
#include <QBuffer>
#include <QMutex>
#include <QFileInfo>
#include <QtXml>
#include <QtConcurrent>
#include <QMap>
#include <QNetworkRequest>
#include <QNetworkReply>

#include "../Core/titleinfo.h"
#include "../Core/settings.h"

#if defined(CEMUDB_LIBRARY)
#  define CEMUDBSHARED_EXPORT Q_DECL_EXPORT
#else
#  define CEMUDBSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // CEMUDB_GLOBAL_H
