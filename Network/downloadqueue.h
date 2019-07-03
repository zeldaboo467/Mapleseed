#ifndef NETWORK_H
#define NETWORK_H

#include "queueinfo.h"
#include "network_global.h"
class NETWORKSHARED_EXPORT DownloadQueue : public QObject
{
    Q_OBJECT
public:
    explicit DownloadQueue();

    static DownloadQueue *initialize();

    void StartQueue();

    void add(QueueInfo *info);

    bool exists(QueueInfo *info);

    bool DownloadSingle(QUrl url, QString filepath, QueueInfo *info);

    static DownloadQueue *instance;

signals:
    void OnEnqueue(QueueInfo *info);
    void OnDequeue(QueueInfo *info);
    void QueueFinished(QList<QueueInfo*> history);
    void DownloadProgress(qint64 received, qint64 total, QTime time);

public slots:
    void progress(qint64 received, qint64 total);

private:
    QList<QueueInfo*> history;
    QQueue<QueueInfo*> queue;
    QTime downloadTime;
};

#endif // NETWORK_H
