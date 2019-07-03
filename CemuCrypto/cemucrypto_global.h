#ifndef CEMUCRYPTO_GLOBAL_H
#define CEMUCRYPTO_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QObject>
#include <QMessageBox>
#include <QtConcurrent>
#include <QDir>
#include <QFile>
#include <QDataStream>

#include <openssl\aes.h>
#include <openssl\sha.h>

#if defined(CEMUCRYPTO_LIBRARY)
#  define CEMUCRYPTOSHARED_EXPORT Q_DECL_EXPORT
#else
#  define CEMUCRYPTOSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // CEMUCRYPTO_GLOBAL_H
