#ifndef CEMULIBRARY_GLOBAL_H
#define CEMULIBRARY_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QObject>
#include <QtDebug>
#include <QBuffer>
#include <QMutex>
#include <QFileInfo>
#include <QtXml>
#include <QtConcurrent>
#include <QNetworkRequest>
#include <QNetworkReply>

#include "../Core/titleinfo.h"
#include "../Core/settings.h"

#if defined(CEMULIBRARY_LIBRARY)
#  define CEMULIBRARYSHARED_EXPORT Q_DECL_EXPORT
#else
#  define CEMULIBRARYSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // CEMULIBRARY_GLOBAL_H
