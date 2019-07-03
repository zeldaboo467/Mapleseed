#ifndef HELPER_H
#define HELPER_H

#include <QFileDialog>
#include <QtDebug>
#include <QListWidget>
#include <QtConcurrent>
#include <QLabel>
#include <QApplication>
#include <QClipboard>
#include "settings.h"
#include "downloadqueue.h"
#include "cemulibrary.h"
#include "cemudatabase.h"
#include "cemucrypto.h"
#include "QtCompressor.h"

class Helper
{
public:
    static float CalculatePrecent(qint64 min, qint64 max)
    {
        return (static_cast<float>(min) / static_cast<float>(max)) * 100;
    }

    static QString CalculateSpeed(qint64 received, QTime qtime)
    {
        double speed = received * 1000.0 / qtime.elapsed();
        QString unit;
        if (speed < 1024) {
          unit = "bytes/sec";
        } else if (speed < 1024 * 1024) {
          speed /= 1024;
          unit = "kB/s";
        } else {
          speed /= 1024 * 1024;
          unit = "MB/s";
        }

        return QString::fromLatin1("%1 %2").arg(speed, 3, 'f', 1).arg(unit);
    }

    static void CopyToClipboard(QString text)
    {
        QApplication::clipboard()->setText(text);
        qInfo() << text << "copied to clipboard";
    }

    static QDir* SelectDirectory()
    {
      QFileDialog dialog;
      dialog.setFileMode(QFileDialog::DirectoryOnly);
      dialog.setOption(QFileDialog::ShowDirsOnly);
      if (dialog.exec())
      {
        QStringList entries(dialog.selectedFiles());
        return new QDir(entries.first());
      }
      return nullptr;
    }

    static QFileInfo SelectFile(QString defaultDir)
    {
        QFileDialog dialog;
        if (QDir(defaultDir).exists())
        {
            dialog.setDirectory(defaultDir);
        }
        dialog.setNameFilter("*.qta");
        if (dialog.exec())
        {
            QStringList entries(dialog.selectedFiles());
            return entries.first();
        }
        return QFileInfo();
    }

    static void filter(QString region, QString filter_string, QListWidget *listWidget)
    {
        if (!filter_string.isEmpty())
            qInfo() << "filter:" << filter_string;

        auto all = listWidget->findItems("*", Qt::MatchFlag::MatchWildcard);
        QtConcurrent::blockingMapped(all, HideListWidgetItem);

        QString searchString;
        auto matches = QList<QListWidgetItem*>();

        if (filter_string.isEmpty()) {
            searchString.append("*" + region + "* *");
            matches = listWidget->findItems(searchString, Qt::MatchFlag::MatchWildcard | Qt::MatchFlag::MatchCaseSensitive);
        }
        else {
            searchString.append("*" + region + "*" + filter_string + "*");
            matches = listWidget->findItems(searchString, Qt::MatchFlag::MatchWildcard);
        }

        QtConcurrent::blockingMapped(matches, ShowListWidgetItem);
    }

    static QListWidgetItem* HideListWidgetItem(QListWidgetItem *item)
    {
        if (mutex().tryLock(100))
        {
            item->setHidden(true);
            mutex().unlock();
        }
        else {
            item->setHidden(true);
        }
        return item;
    }

    static QListWidgetItem* ShowListWidgetItem(QListWidgetItem *item)
    {
        if (mutex().tryLock(100))
        {
            item->setHidden(false);
            mutex().unlock();
        }
        else {
            item->setHidden(false);
        }
        return item;
    }

    static void SelectionChanged(QListWidget* listWidget, QLabel *label)
    {
        auto items = listWidget->selectedItems();
        if (items.isEmpty())
            return;

        auto id(items.first()->data(Qt::UserRole).toString());
        auto info = findWiiUTitleInfo(id);

        label->setPixmap(QPixmap(info->coverArt()));
    }

    static inline QMutex &mutex()
    {
        static QMutex mutex;
        return mutex;
    }

    static QString fomartSize(float size)
    {
        float num = size;
        QStringList list;
        list << "KB" << "MB" << "GB" << "TB";

        QStringListIterator i(list);
        QString unit("bytes");

        while (num >= 1024.0 && i.hasNext())
        {
            unit = i.next();
            num /= 1024.0;
        }
        return QString().setNum(num, 'f', 2) + " " + unit;
    }

    static TitleInfo *findWiiUTitleInfo(QString id)
    {
        TitleInfo *info;
        if ((info = CemuLibrary::find(id))) {
            return info;
        }
        if ((info = CemuDatabase::find(id))) {
            return info;
        }
        return nullptr;
    }

    static QueueInfo *GetWiiuDownloadInfo(QString id, QString version)
    {
        auto info = Helper::findWiiUTitleInfo(id);

        QString baseURL("http://ccs.cdn.wup.shop.nintendo.net/ccs/download/");
        if (info->key().isEmpty() || info->key().length() != 32) {
            qWarning() << "Invalid title key" << info->key();
            return nullptr;
        }

        QString directory(info->dir());
        if (!QDir(directory).exists())
        {
            QDir().mkpath(directory);
        }

        auto tmdData = CemuDatabase::DownloadTMD(id, version, directory);
        auto tmd = reinterpret_cast<CemuCrypto::TitleMetaData*>(tmdData);
        CemuDatabase::CreateTicket(id, info->key(), version, directory);

        if (!tmd) {
            qCritical() << "Invalid TMD. Cannot continue!";
            return nullptr;
        }

        auto contentCount = CemuCrypto::bs16(tmd->ContentCount);
        if (contentCount > 1024)
            return nullptr;

        auto qinfo = new QueueInfo;
        qinfo->name = info->formatName();
        qinfo->directory = directory;
        qinfo->totalSize = 0;
        for (int i = 0; i < contentCount; i++)
        {
            QString contentID = QString().sprintf("%08x", CemuCrypto::bs32(tmd->Contents[i].ID));
            QString contentPath = QDir(directory).filePath(contentID);
            QString downloadURL = baseURL + info->id().toUpper() + QString("/") + contentID;
            qulonglong size = CemuCrypto::bs64(tmd->Contents[i].Size);
            if (!QFile(contentPath).exists() || QFileInfo(contentPath).size() != static_cast<qint64>(size))
            {
                qinfo->totalSize += size;
                qinfo->urls.push_back({contentPath,downloadURL});
            }
        }

        return qinfo;
    }

    static QString CemuSaveDir(QString id)
    {
        QString path = Settings::value("cemu/path").toString();
        QString cemupath = QFileInfo(path).absoluteDir().path();
        if (!QDir(cemupath).exists())
        {
            qWarning() << "Cemu path not found:" << cemupath;
            return nullptr;
        }
        QString savedir = QDir(cemupath).filePath("mlc01/usr/save/00050000");
        if (!QDir(savedir).exists())
        {
            qWarning() << "Cemu default save path not found:" << savedir;
            return nullptr;
        }
        savedir.append("/" + id.toUpper().right(8));
        if (!QDir(savedir).exists())
        {
            qWarning() << "Save data not found:" << savedir;
            return nullptr;
        }
        return savedir;
    }

    static void BackupCemuSave(QString id, QString saveTo, QtCompressor *compressor)
    {
        auto tinfo = findWiiUTitleInfo(id);
        saveTo = QDir(saveTo).filePath(tinfo->formatName());
        QDir().mkpath(saveTo);
        saveTo = QDir(saveTo).filePath(id.right(8) + "-" + QDateTime::currentDateTime().toString("MM-dd-yyyy hh.mm.ss AP") + ".qta");
        QtConcurrent::run([=]
        {
            QString savedir = CemuSaveDir(id);
            if (savedir.isEmpty())
            {
                return;
            }
            compressor->compress(savedir, saveTo);
            qInfo() << "Backup Saved:" << saveTo;
        });
    }

    static void ImportCemuSave(QString filePath, QtCompressor *compressor)
    {
        QString filename = QFileInfo(filePath).fileName();
        QString id = "00050000" + filename.left(8);
        if (!CemuDatabase::find(id))
        {
            qCritical() << "invalid filename, can not detect title id" << filename;
            return;
        }
        QString savedir = CemuSaveDir(id);
        if (savedir.isEmpty())
        {
            qCritical() << "Save directory is invalid or doesn't exist:" << savedir;
            qCritical() << "Save directories are not created by MS, this is to avoid import issues.";
            qCritical() << "Try creating a save file by playing the game through Cemu, then import again.";
            return;
        }
        QtConcurrent::run([=]
        {
            compressor->decompress(filePath, QDir(savedir).absolutePath());
            qInfo() << "Backup imported:" << QDir(savedir).absolutePath();
        });
    }

};

#endif // HELPER_H
