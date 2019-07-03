#include "logging.h"
#include "settings.h"

Logging *Logging::instance;

Logging::Logging(QObject *parent) : QObject(parent)
{
    connect(this, &Logging::OnLogEvent, this, &Logging::logEvent);
    Logging::instance = this;
}

void Logging::categoryFilter(QLoggingCategory *category)
{
    if (strcmp(category->categoryName(), "default") == 0)
    {
        category->setEnabled(QtDebugMsg, Settings::value("debug").toBool());
    }
    else if (strcmp(category->categoryName(), "qt.gamepad") == 0)
    {
        category->setEnabled(QtDebugMsg, Settings::value("debug").toBool());
    }
    else
    {
        category->setEnabled(QtDebugMsg, false);
    }
}

void Logging::messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    const char *file = context.file ? context.file : "";
    const char *function = context.function ? context.function : "";
    QString qtype;
    switch (type)
    {
    case QtDebugMsg:
        qtype = "Debug";
        break;
    case QtInfoMsg:
        qtype = "Info";
        break;
    case QtWarningMsg:
        qtype = "Warning";
        break;
    case QtCriticalMsg:
        qtype = "Critical";
        break;
    case QtFatalMsg:
        qtype = "Fatal";
        break;
    }
#ifdef QT_DEBUG
    emit instance->OnLogEvent(QString("%1: %2 (%3:%4, %5)")
                                .arg(qtype)
                                .arg(localMsg.constData())
                                .arg(file)
                                .arg(context.line)
                                .arg(function));
#else
    emit instance->OnLogEvent(QString("%1: %2").arg(qtype).arg(localMsg.constData()));
#endif
}

void Logging::logEvent(QString msg)
{
    if (Settings::value("debug").toBool())
    {
        QFile file("mapleseed.log");
        if (!file.open(QIODevice::Append))
        {
          qWarning("Couldn't open file.");
          return;
        }
        QString log(QDateTime::currentDateTime().toString("[MMM dd, yyyy HH:mm:ss ap] ") + msg + "\n");
        file.write(log.toLatin1());
        file.close();
    }
}
