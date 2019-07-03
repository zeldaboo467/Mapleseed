#ifndef NETWORK_GLOBAL_H
#define NETWORK_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QObject>
#include <QtDebug>
#include <QBuffer>
#include <QMutex>
#include <QFileInfo>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QMap>
#include <QTimer>
#include <QProgressBar>
#include <QTime>
#include <QQueue>
#include <QUrl>
#include <QEventLoop>

#if defined(NETWORK_LIBRARY)
#  define NETWORKSHARED_EXPORT Q_DECL_EXPORT
#else
#  define NETWORKSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // NETWORK_GLOBAL_H
