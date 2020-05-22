#include <QtWidgets>

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

#ifdef _WIN32
    ui->pushButton_ParentDir->setIcon(QPixmap(":/images/up.png"));
    ui->actionExit->setIcon(QPixmap(":/images/close.png"));
#endif

    // Wyłączenie przycisków przed połączeniem z serwerem
    ui->pushButton_DownloadFile->setEnabled(false);
    ui->treeWidget->setEnabled(false);
    ui->pushButton_ParentDir->setEnabled(false);

    // Ustawienie nagłówków i możliwości sortowania w TreeWidget
    ui->treeWidget->setRootIsDecorated(false);
    ui->treeWidget->setHeaderLabels(QStringList() << "Nazwa" << "Rozmiar" << "Data modyfikacji");
    ui->treeWidget->header()->setStretchLastSection(false);
    ui->treeWidget->setSortingEnabled(true);

    // Zmienna określająca, czy nawiązano połączenie z serwerem
    isAlreadyConnected = false;

    // Łączenie sygnałów i slotów
    connect(ui->treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT(enableDownload()));
    connect(ui->pushButton_Connect, SIGNAL(clicked()), this, SLOT(connectDisconnect()));
    connect(ui->pushButton_DownloadFile, SIGNAL(clicked()), this, SLOT(downloadFile()));
    connect(ui->pushButton_ParentDir, SIGNAL(clicked()), this, SLOT(goToParent()));
    connect(ui->pushButton_Close, SIGNAL(clicked()), this, SLOT(closeApp()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Łączenie bądź rozłączanie
void MainWindow::connectDisconnect()
{
    if (isAlreadyConnected)
        disconnectFTP();
    else
        connectToFTP();
}

// Łączenie z serwerem FTP
void MainWindow::connectToFTP()
{
    ftp = new QFtp(this);
    connect(ftp, SIGNAL(commandFinished(int, bool)), this, SLOT(ftpCommandFinished(int, bool)));
    connect(ftp, SIGNAL(listInfo(QUrlInfo)), this, SLOT(addToList(QUrlInfo)));
    connect(ui->treeWidget, SIGNAL(itemActivated(QTreeWidgetItem*, int)), this, SLOT(processItem(QTreeWidgetItem*, int)));

    ui->treeWidget->clear();
    isDirectory.clear();

    bool isAddress = true, isUsername = true;

    // Walidacja pola z adresem serwera
    QString address(ui->lineEdit_address->text());
    if (address == "") {
        ui->statusBar->showMessage("Adres serwera jest wymagany.", 2000);
        isAddress = false;
    }

    QString port_str = ui->lineEdit_port->text();
    qint16 port = 21;
    if (port_str != "")
        port = port_str.toInt();

    // Walidacja pola z nazwą użytkownika
    QString username = ui->lineEdit_username->text();
    if (username == "") {
        ui->statusBar->showMessage("Nazwa użytkownika jest wymagana.", 2000);
        isUsername = false;
    }

    QString password = ui->lineEdit_password->text();

    if (isAddress && isUsername) {
        ftp->connectToHost(address, port);
        ftp->login(username, password);
        ui->treeWidget->setEnabled(true);
    } else {
        QString message("Nie można kontynuować, ponieważ:");
        if (!isAddress)
            message += "<br><br><b>Nie podano adresu serwera.</b>";
        if (!isUsername)
            message += "<br><br><b>Nie podano nazwy użytkownika.</b>";
        QMessageBox::warning(this, "Qt FTP Client", message);
    }
}

// Rozłączanie z FTP
void MainWindow::disconnectFTP()
{
    if (ftp) {
        ftp->abort();
        ftp->deleteLater();
        ftp->close();
        ftp = 0;

        // Resetowanie TreeView i powiązanych z nim elementów
        ui->statusBar->showMessage("Pomyślnie wylogowano.");
        isAlreadyConnected = false;
        isDirectory.clear();
        ui->treeWidget->clear();
        ui->treeWidget->setEnabled(false);

        // Resetowanie stanu przycisków
        ui->pushButton_ParentDir->setEnabled(false);
        ui->pushButton_DownloadFile->setEnabled(false);
        ui->pushButton_Connect->setText("Połącz");
    }
}

// Dodawanie plików do listy
void MainWindow::addToList(const QUrlInfo &urlInfo)
{
    QTreeWidgetItem *item = new QTreeWidgetItem;
    item->setText(0, urlInfo.name());
    item->setText(1, QString::number(urlInfo.size()));
    item->setText(2, urlInfo.lastModified().toString("dd.MM.yyyy"));

    // Wyświetlanie ikony folderu bądź pliku w zależności od rodzaju elementu
    QPixmap pixmap(urlInfo.isDir() ? ":/images/dir.png" : ":/images/file.png");
    item->setIcon(0, pixmap);

    isDirectory[urlInfo.name()] = urlInfo.isDir();
    ui->treeWidget->addTopLevelItem(item);
    if (!ui->treeWidget->currentItem()) {
        ui->treeWidget->setCurrentItem(ui->treeWidget->topLevelItem(0));
        ui->treeWidget->setEnabled(true);
    }
}

// Przejście do katalogu
void MainWindow::processItem(QTreeWidgetItem *item, int)
{
    QString name = item->text(0);
    if (isDirectory.value(name)) {
        ui->treeWidget->clear();
        isDirectory.clear();
        currentPath += '/';
        currentPath += name;
        ftp->cd(name);
        ftp->list();
        ui->pushButton_ParentDir->setEnabled(true);
        return;
    }
}

// Przejście do katalogu nadrzędnego
void MainWindow::goToParent()
{
    ui->treeWidget->clear();
    isDirectory.clear();
    currentPath = currentPath.left(currentPath.lastIndexOf('/'));
    if (currentPath.isEmpty()) {
        ui->pushButton_ParentDir->setEnabled(false);
        ftp->cd("/");
    } else {
        ftp->cd(currentPath);
    }
    ftp->list();
}

// Obsługa komend FTP
void MainWindow::ftpCommandFinished(int, bool error)
{
    switch (ftp->currentCommand()) {
    case QFtp::ConnectToHost:
        if (error) {
            ui->statusBar->showMessage("Nie można połączyć się z serwerem.");
            QMessageBox::warning(this, "Qt FTP Client", tr("Nie można połączyć się z serwerem.<br>Serwer pod adresem %1:%2 jest nieosiągalny.").arg(ui->lineEdit_address->text(), ui->lineEdit_port->text()));
            return;
        }
        ui->statusBar->showMessage("Pomyślnie połączono.");
        break;

    case QFtp::Login:
        if (error) {
            ui->statusBar->showMessage("Nie można się zalogować.");
            QMessageBox::warning(this, "Qt FTP Client", "Błędna nazwa użytkownika lub hasło.");
        } else {
            isAlreadyConnected = true;
            ui->pushButton_Connect->setText("Rozłącz");
            ui->pushButton_DownloadFile->setEnabled(true);
            ui->statusBar->showMessage("Pomyślnie zalogowano.");
            ftp->list();
        }
        break;

    case QFtp::Get:
        if (error) {
            ui->statusBar->showMessage("Nie można pobrać pliku.");
            file->close();
            file->remove();
        } else {
            ui->statusBar->showMessage(tr("Pomyślnie pobrano plik %1.").arg(file->fileName()));
            file->close();
        }
        delete file;
        enableDownload();

        break;

    case QFtp::List:
        if (isDirectory.isEmpty()) {
            ui->treeWidget->addTopLevelItem(new QTreeWidgetItem(QStringList() << "<pusty katalog>"));
            ui->treeWidget->setEnabled(false);
        }
        break;

    default:
        ;
    }
}

// Włączenie pobierania, w zależności od tego, czy bieżącym elementem jest plik
void MainWindow::enableDownload()
{
    QTreeWidgetItem *current = ui->treeWidget->currentItem();
    if (current) {
        QString currentFile = current->text(0);
        ui->pushButton_DownloadFile->setEnabled(!isDirectory.value(currentFile));
    } else {
        ui->pushButton_DownloadFile->setEnabled(false);
    }
}

// Pobieranie pliku
void MainWindow::downloadFile()
{
    // Wyświetlanie okna dialogowego zapisu pliku
    QString fileName = ui->treeWidget->currentItem()->text(0);
    QString fileToDownload = QFileDialog::getSaveFileName(this, "Zapisz plik jako...", fileName);

    file = new QFile(fileToDownload);
    if (!file->open(QIODevice::WriteOnly)) {
        QMessageBox::information(this, "Qt FTP Client", "Nie można zapisać pliku");
        delete file;
        return;
    }

    ftp->get(ui->treeWidget->currentItem()->text(0), file);
}

// Informacje o programie
void MainWindow::aboutApp()
{
    QString aboutInformation("<b>Qt FTP Client</b><br><br>Prosty klient FTP oparty o framework Qt.<br><br>Autorzy:<br><br><b>Daniel Mikołajczewski<br>Krystian Łężniak</b>");
    QMessageBox::about(this, "O programie", aboutInformation);
}

// Zamknięcie programu
void MainWindow::closeApp()
{
    QApplication::quit();
}


// Wyświetlenie informacji o programie
void MainWindow::on_actionAboutApp_triggered()
{
    aboutApp();
}

// Zamknięcie programu z menu Plik -> Wyjście
void MainWindow::on_actionExit_triggered()
{
    closeApp();
}

// Wyświetlenie informacji o Qt
void MainWindow::on_actionAboutQt_triggered()
{
    QApplication::aboutQt();
}

