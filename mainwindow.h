#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHash>
#include <QtNetwork>
#include <QNetworkConfigurationManager>
#include <QFtp>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QFile;
class QFtp;
class QUrlInfo;
class QTreeWidgetItem;
class QNetworkSession;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void connectDisconnect();
    void downloadFile();
    void connectToFTP();
    void disconnectFTP();

    void ftpCommandFinished(int command, bool error);
    void addToList(const QUrlInfo &urlInfo);
    void processItem(QTreeWidgetItem *item, int column);
    void goToParent();
    void enableDownload();

    void closeApp();
    void aboutApp();

    void on_actionAboutApp_triggered();

    void on_actionExit_triggered();

    void on_actionAboutQt_triggered();

private:
    Ui::MainWindow *ui;

    QHash<QString, bool> isDirectory;
    QString currentPath;
    QFtp *ftp;
    QFile *file;

    QNetworkSession *networkSession;
    QNetworkConfigurationManager manager;

    bool isAlreadyConnected;
};
#endif // MAINWINDOW_H
