#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QtConcurrent>
#include <QDesktopServices>
#include <QFileDialog>
#include <QListWidget>
#include "gamepad.h"
#include "cemudatabase.h"
#include "cemulibrary.h"
#include "queueinfo.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void initialize();

    void setupConnections();

    void downloadCemuId(QString id, QString ver);

    void executeCemu(QString rpxPath);

    bool processActive();

private slots:
      void logEvent(QString msg);

      void downloadQueueAdd(QueueInfo *info);

      void updateDownloadProgress(qint64 bytesReceived, qint64 bytesTotal, QTime qtime);

      void updateCemuCryptoProgress(int curfile, int maxfiles);

      void on_showContextMenu(QListWidget* listWidget, const QPoint& pos);

      void CemuDbLoadComplete();

      void NewDatabaseEntry(TitleInfo*info);

      void NewLibraryEntry(QString xmlfile);

      void on_actionExit_triggered();

      void on_actionDebug_triggered(bool checked);

      void on_actionClearSettings_triggered();

      void on_actionOpenLog_triggered();

      void on_actionCemuFullscreen_triggered(bool checked);

      void on_actionCemuIntegrate_triggered(bool checked);

      void on_actionCemuChangeLibrary_triggered();

      void on_actionCemuRefreshLibrary_triggered();

      void on_searchInput_textEdited(const QString &arg1);

      void on_regionBox_currentTextChanged(const QString &arg1);

      void on_libraryListWidget_itemSelectionChanged();

      void on_databaseListWidget_itemSelectionChanged();

      void on_libraryListWidget_customContextMenuRequested(const QPoint &pos);

      void on_databaseListWidget_customContextMenuRequested(const QPoint &pos);

      void on_actionCemuDecrypt_triggered();

      void gameUp(bool pressed);

      void gameDown(bool pressed);

      void gameStart(bool pressed);

      void gameClose(bool pressed);

      void prevTab(bool pressed);

      void nextTab(bool pressed);

      void on_actionGamepad_triggered(bool checked);

private:
    Ui::MainWindow *ui;
    QMutex mutex;
    QProcess* process = new QProcess;
};

#endif // MAINWINDOW_H
