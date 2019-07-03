#ifndef QUEUEINFO_H
#define QUEUEINFO_H

#include "network_global.h"
class NETWORKSHARED_EXPORT QueueInfo : public QObject
{
    Q_OBJECT
public:
    explicit QueueInfo(QObject *parent = nullptr) : QObject(parent)
    {
        pgbar.setStyleSheet("QProgressBar {\nborder: 1px solid black;\ntext-align: center;\npadding: 1px;\nwidth: 15px;\n}\n\nQProgressBar::chunk {\nbackground-color: #cd9bff;\nborder: 1px solid black;\n}");
        pgbar.setAlignment(Qt::AlignmentFlag::AlignHCenter | Qt::AlignmentFlag::AlignVCenter);
        pgbar.setRange(0, 100);
    }
    ~QueueInfo()
    {
        //delete reply;
    }

    QList<QPair<QString, QUrl>> urls;
    QString name;
    QString directory;
    qint64 totalSize = 0;
    qint64 bytesReceived = 0;
    QProgressBar pgbar;
    QVariant userData;
    QNetworkReply *reply;
    QFile file;

signals:
    void finished();

public slots:
    void readyRead()
    {
        if (!file.isWritable()) return;
        if (!reply->isReadable()) return;
        auto qbyteArray = reply->readAll();
        auto written = file.write(qbyteArray);
        updateProgress(bytesReceived += written, totalSize);
    }
    void updateProgress(qint64 received, qint64 total)
    {
        float percent = (static_cast<float>(received) / static_cast<float>(total)) * 100;
        pgbar.setValue(static_cast<int>(percent));
    }
};

#endif // QUEUEINFO_H
