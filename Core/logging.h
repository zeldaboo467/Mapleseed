#ifndef LOGGING_H
#define LOGGING_H

#include <QObject>
#include <QtDebug>
#include <QLoggingCategory>
#include <QCoreApplication>
#include <QFile>
#include <QDateTime>

class Logging : public QObject
{
    Q_OBJECT
public:
    explicit Logging(QObject *parent = nullptr);
    ~Logging();

    static Logging *initialize();

    static void categoryFilter(QLoggingCategory *category);

    static void messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);

    static Logging *instance;

private:
    QFile *file;

signals:
    void OnLogEvent(QString msg);

public slots:
    void logEvent(QString msg);
};

#endif // LOGGING_H
