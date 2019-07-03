#include "downloadqueue.h"

DownloadQueue *DownloadQueue::instance;

DownloadQueue::DownloadQueue()
{
}

DownloadQueue *DownloadQueue::initialize()
{
    qInfo() << "Network Library v1.0.0 loaded";

    if (!instance) {
        instance = new DownloadQueue;
    }

    return instance;
}

void DownloadQueue::StartQueue()
{
    if (queue.isEmpty())
    {
        emit QueueFinished(history);
        history.clear();
        return;
    }

    downloadTime.start();
    auto qinfo = queue.first();

    for (auto pair : qinfo->urls)
    {
        QString filepath(pair.first);
        QUrl url(pair.second);
        DownloadSingle(url, filepath, qinfo);
    }

    history.append(qinfo);
    emit qinfo->finished();
    emit OnDequeue(queue.dequeue());
    qInfo() << "Remove from Queue " << qinfo->name;

    StartQueue();
}

void DownloadQueue::add(QueueInfo *info)
{
    if (queue.isEmpty()) {
        QTimer::singleShot(0, Qt::CoarseTimer, this, &DownloadQueue::StartQueue);
    }

    queue.enqueue(info);
    emit OnEnqueue(info);
    qInfo() << "Add to Queue '" << info->name;
}

bool DownloadQueue::exists(QueueInfo *info)
{
    bool result = false;
    for(auto item : queue)
    {
        if (item->name == info->name){
            result = true;
            break;
        }
    }
    return result;
}

bool DownloadQueue::DownloadSingle(QUrl url, QString filepath, QueueInfo *info)
{
    auto file = &info->file;

    file->setFileName(filepath);
    if (!file->open(QIODevice::WriteOnly)) {
        qCritical() << file->errorString();
        return false;
    }

    QNetworkAccessManager manager;
    QNetworkRequest request(url);

    QEventLoop loop;
    info->reply = manager.get(request);
    connect(info->reply, &QNetworkReply::downloadProgress, this, &DownloadQueue::progress);
    connect(info->reply, &QNetworkReply::readyRead, info, &QueueInfo::readyRead);
    connect(info->reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    file->close();
    return true;
}

void DownloadQueue::progress(qint64 received, qint64 total)
{
    emit DownloadProgress(received, total, downloadTime);
}
