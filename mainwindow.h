#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void closeApp();
    void aboutApp();
    void connectToServer();

    void on_actionAboutApp_triggered();

    void on_actionExit_triggered();

    void on_actionAboutQt_triggered();

    void on_treeView_clicked(const QModelIndex &index);

private:
    Ui::MainWindow *ui;

    void createActions();
};
#endif // MAINWINDOW_H
