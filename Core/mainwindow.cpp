#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "logging.h"
#include "helper.h"
#include "settings.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initialize();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initialize()
{
    setWindowTitle("MapleSeed++ v1.1.0");

    connect(Logging::instance, &Logging::OnLogEvent, this, &MainWindow::logEvent);

    ui->actionDebug->setChecked(Settings::value("debug").toBool());
    ui->actionCemuIntegrate->setChecked(Settings::value("cemu/enabled").toBool());
    ui->actionCemuFullscreen->setChecked(Settings::value("cemu/fullscreen").toBool());

    if (Settings::value("cemu/coversDir").toString().isEmpty()) {
        Settings::setValue("cemu/coversDir", QDir(Settings::getdirpath()).filePath("covers"));
    }

    Settings settings;
    CemuDatabase::instance = new CemuDatabase;
    CemuLibrary::instance = new CemuLibrary;
    DownloadQueue::instance = new DownloadQueue;

    initConnections();
    CemuDatabase::initialize();
    CemuLibrary::initialize();
    DownloadQueue::initialize();
}

void MainWindow::initConnections()
{
    qDebug() << "initializing event connections";
    connect(CemuDatabase::instance, &CemuDatabase::OnNewEntry, this, &MainWindow::on_newDatabase_entry);
    connect(CemuDatabase::instance, &CemuDatabase::OnLoadComplete, this, &MainWindow::on_CemuDb_LoadComplete);
    connect(CemuLibrary::instance, &CemuLibrary::OnNewEntry, this, &MainWindow::on_newLibrary_entry);
    connect(DownloadQueue::instance, &DownloadQueue::OnEnqueue, this, &MainWindow::downloadQueueAdd);
    connect(DownloadQueue::instance, &DownloadQueue::DownloadProgress, this, &MainWindow::updateDownloadProgress);
}

void MainWindow::downloadCemuId(QString id, QString ver)
{
    auto tinfo = Helper::findWiiUTitleInfo(id);
    auto qinfo = Helper::GetWiiuDownloadInfo(id, ver);
    auto crypto = CemuCrypto::initialize(tinfo->key(), qinfo->directory);
    auto watcher = new QFutureWatcher<void>;
    auto row = qinfo->userData.toInt();

    if (!DownloadQueue::instance->exists(qinfo))
    {
        DownloadQueue::instance->add(qinfo);
    }

    connect(watcher, &QFutureWatcher<void>::finished, this, [=]
    {
        disconnect(crypto, &CemuCrypto::Progress, qinfo, &QueueInfo::updateProgress);
        delete watcher;
        delete qinfo;
        delete crypto;
        ui->downloadQueueTableWidget->removeRow(row);
        ui->downloadQueueTableWidget->horizontalHeader()->setStretchLastSection(true);
        ui->downloadQueueTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    });

    connect(qinfo, &QueueInfo::finished, [=]
    {
        connect(crypto, &CemuCrypto::Progress, qinfo, &QueueInfo::updateProgress);
        watcher->setFuture(QtConcurrent::run(crypto, &CemuCrypto::Start));
    });
}

void MainWindow::logEvent(QString msg)
{
    if (mutex.tryLock(100))
    {
        if (ui && ui->statusbar)
        {
            ui->statusbar->showMessage(msg);
        }
        mutex.unlock();
    }
}

void MainWindow::downloadQueueAdd(QueueInfo *info)
{
    info->userData = ui->downloadQueueTableWidget->rowCount();
    int row = info->userData.toInt();
    ui->downloadQueueTableWidget->insertRow(row);
    ui->downloadQueueTableWidget->setCellWidget(row, 0, new QLabel(info->name));
    ui->downloadQueueTableWidget->setCellWidget(row, 1, new QLabel(Helper::fomartSize(info->totalSize)));
    ui->downloadQueueTableWidget->setCellWidget(row, 2, &info->pgbar);
    ui->downloadQueueTableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->downloadQueueTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void MainWindow::updateDownloadProgress(qint64 bytesReceived, qint64 bytesTotal, QTime qtime)
{
    float percent = Helper::CalculatePrecent(bytesReceived, bytesTotal);
    auto speed = Helper::CalculateSpeed(bytesReceived, qtime);

    ui->progressBar->setRange(0, 100);
    ui->progressBar->setValue(static_cast<int>(percent));
    ui->progressBar->setFormat("%p% " + Helper::fomartSize(bytesReceived) + " / " + Helper::fomartSize(bytesTotal) + " | " + speed);
}

void MainWindow::updateCemuCryptoProgress(int curfile, int maxfiles)
{
    float per = (static_cast<float>(curfile) / static_cast<float>(maxfiles)) * 100;
    this->ui->progressBar->setRange(0, 100);
    this->ui->progressBar->setValue(static_cast<int>(per));
    this->ui->progressBar->setFormat(QString::number(static_cast<double>(per), 'G', 3) + "% | " +
        QString::number(curfile) + " / " + QString::number(maxfiles) + " files");
}

void MainWindow::on_showContextMenu(QListWidget *listWidget, const QPoint &pos)
{
    QPoint globalPos = listWidget->mapToGlobal(pos);
    if (listWidget->selectedItems().isEmpty()) {
      return;
    }
    auto item = listWidget->selectedItems().first();
    QString id = item->data(Qt::UserRole).toString();
    auto info = Helper::findWiiUTitleInfo(id);
    if (!info){
        return;
    }

    QString name(QFileInfo(info->dir()).baseName());
    if (name.isEmpty()) {
        name = info->name();
    }

    QMenu menu;

    menu.addAction(name, [=]{})->setEnabled(false);

    menu.addSeparator();
    if (CemuDatabase::ValidId(info->id().replace(7, 1, '0'))) {
        menu.addAction("Download Game", this, [&]
        {
            downloadCemuId(info->id().replace(7, 1, '0'), "");
        });
    }
    if (CemuDatabase::ValidId(info->id().replace(7, 1, 'c'))) {
        menu.addAction("Download DLC", this, [&]
        {
            downloadCemuId(info->id().replace(7, 1, 'c'), "");
        });
    }
    if (CemuDatabase::ValidId(info->id().replace(7, 1, 'e'))) {
        menu.addAction("Download Patch", this, [&]
        {
            downloadCemuId(info->id().replace(7, 1, 'e'), "");
        });
    }

    if (QFileInfo(info->Rpx()).exists() && Settings::value("cemu/enabled").toBool())
    {
        QtCompressor compressor;
        menu.addSeparator();
        menu.addAction("Export Save Data", this, [&]
        {
            QDir dir = Settings::value("cemu/library").toString();
            if (dir.exists())
            {
                Helper::BackupCemuSave(info->id(), dir.filePath("Backup"), &compressor);
            }
            else
            {
                qWarning() << "Save data export failed, base directory not valid" << dir;
            }
        })->setEnabled(true);
        menu.addAction("Import Save Data", this, [&]
        {
            auto baseDir = Settings::value("cemu/library").toString();
            QString dir = QDir(baseDir+"/Backup/"+info->formatName()).absolutePath();
            if (QDir().mkpath(dir))
            {
                QFileInfo fileInfo = Helper::SelectFile(dir);
                if (fileInfo.exists())
                {
                    Helper::ImportCemuSave(fileInfo.absoluteFilePath(), &compressor);
                }
            }
        })->setEnabled(true);
        menu.addAction("Purge Save Data", this, [&]
        {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, info->formatName(), "Purge Save Data?", QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::Yes)
            {
                QString saveDir = Helper::CemuSaveDir(info->id());
                QDir meta = QDir(saveDir).filePath("meta");
                QDir user = QDir(saveDir).filePath("user");
                if (!meta.removeRecursively() || !user.removeRecursively())
                {
                    qWarning() << "Purge Save Data: failed";
                }
            }
        })->setEnabled(true);
    }

    menu.addSeparator();
    if (QFile(QDir(info->dir()).filePath("tmd")).exists() && QFile(QDir(info->dir()).filePath("cetk")).exists())
    {
        menu.addAction("Decrypt Content", this, [=]
        {
            QtConcurrent::run([=]
            {
                CemuCrypto crypto(info->key(), info->dir());
                connect(&crypto, &CemuCrypto::Progress, this, &MainWindow::updateCemuCryptoProgress);
                crypto.Start();
            });
        });
    }

    menu.addAction("Copy ID to Clipboard", this, [=] { Helper::CopyToClipboard(info->id()); });

    menu.exec(globalPos);
}

void MainWindow::on_CemuDb_LoadComplete()
{
    Helper::filter("USA", "", ui->databaseListWidget);
}

void MainWindow::on_newDatabase_entry(TitleInfo *info)
{
    if (info->titleType() != TitleType::Game) return;

    auto item = new QListWidgetItem();
    item->setData(Qt::DisplayRole, info->formatName());
    item->setData(Qt::UserRole, info->id());
    ui->databaseListWidget->addItem(item);
}

void MainWindow::on_newLibrary_entry(QString xmlfile)
{
    auto info = CemuDatabase::Create(xmlfile);

    if (info->titleType() == TitleType::Game)
    {
        auto item = new QListWidgetItem();
        item->setData(Qt::DisplayRole, info->formatName());
        item->setData(Qt::UserRole, info->id());
        ui->libraryListWidget->addItem(item);

        CemuLibrary::instance->library[info->id()] = std::move(info);
    }
}

void MainWindow::on_actionExit_triggered()
{
    if (QMessageBox::question(this, "Exit", "Exit Program?", QMessageBox::Yes|QMessageBox::No) != QMessageBox::Yes)
        return;
    qApp->closeAllWindows();
}

void MainWindow::on_actionDebug_triggered(bool checked)
{
    Settings::setValue("debug", checked);
}

void MainWindow::on_actionClearSettings_triggered()
{
    if (QMessageBox::question(this, "Confirm", "Clear program settings?\nProgram will exit.", QMessageBox::Yes|QMessageBox::No) != QMessageBox::Yes)
        return;
    Settings::clear();
    qApp->closeAllWindows();
}

void MainWindow::on_actionOpenLog_triggered()
{
    if (QFile("mapleseed.log").exists())
    {
        QDesktopServices::openUrl(QUrl("mapleseed.log"));
    }
}

void MainWindow::on_actionCemuFullscreen_triggered(bool checked)
{
    Settings::setValue("cemu/fullscreen", checked);
}

void MainWindow::on_actionCemuIntegrate_triggered(bool checked)
{
    Settings::setValue("cemu/enabled", checked);
    QString cemupath(Settings::value("cemu/path").toString());
    if (checked && !QFile(cemupath).exists())
    {
      QFileDialog dialog;
      dialog.setNameFilter("cemu.exe");
      dialog.setFileMode(QFileDialog::ExistingFile);
      if (dialog.exec())
      {
        QStringList files(dialog.selectedFiles());
        Settings::setValue("cemu/path", QFileInfo(files.first()).absoluteFilePath());
      }
      else
      {
          ui->actionCemuIntegrate->setChecked(false);
          return;
      }
    }
    if (checked)
    {
        auto str("Save Data exports WILL NOT work with any other save data tool/program. DO NOT change the default export file name.");
        QMessageBox::information(this, "Warning!!!!!!",  str);
    }
}

void MainWindow::on_actionCemuChangeLibrary_triggered()
{
    QDir* dir = Helper::SelectDirectory();
    if (dir == nullptr)
      return;
    QString directory(dir->path());
    delete dir;

    ui->libraryListWidget->clear();
    Settings::setValue("cemu/library", directory);
    QtConcurrent::run([=] { CemuLibrary::instance->init(directory); });
}

void MainWindow::on_actionCemuRefreshLibrary_triggered()
{
    ui->libraryListWidget->clear();
    auto directory(Settings::value("cemu/library").toString());
    QtConcurrent::run([=] { CemuLibrary::instance->init(directory); });
}

void MainWindow::on_searchInput_textEdited(const QString &arg1)
{
    Helper::filter(ui->regionBox->currentText(), arg1, ui->databaseListWidget);
}

void MainWindow::on_regionBox_currentTextChanged(const QString &arg1)
{
    Helper::filter(arg1, ui->searchInput->text(), ui->databaseListWidget);
}

void MainWindow::on_libraryListWidget_itemSelectionChanged()
{
    Helper::SelectionChanged(ui->libraryListWidget, ui->coverLabel);
}

void MainWindow::on_databaseListWidget_itemSelectionChanged()
{
    Helper::SelectionChanged(ui->databaseListWidget, ui->coverLabel);
}

void MainWindow::on_libraryListWidget_customContextMenuRequested(const QPoint &pos)
{
    on_showContextMenu(ui->libraryListWidget, pos);
}

void MainWindow::on_databaseListWidget_customContextMenuRequested(const QPoint &pos)
{
    on_showContextMenu(ui->databaseListWidget, pos);
}

void MainWindow::on_actionCemuDecrypt_triggered()
{
    QDir* dir = Helper::SelectDirectory();
    if (dir == nullptr)
      return;

    if (!QFileInfo(dir->filePath("tmd")).exists()) {
      QMessageBox::critical(this, "Missing file", +"Missing: " + dir->filePath("/tmd"));
      return;
    }
    if (!QFileInfo(dir->filePath("cetk")).exists()) {
      QMessageBox::critical(this, "Missing file", +"Missing: " + dir->filePath("/cetk"));
      return;
    }

    QString path = dir->path();
    delete dir;
    qInfo() << "Decrypting" << path;

    QtConcurrent::run([=]
    {
        CemuCrypto crypto("", path);
        connect(&crypto, &CemuCrypto::Progress, this, &MainWindow::updateCemuCryptoProgress);
        crypto.Start();
    });
}
