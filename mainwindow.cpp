#include <QtWidgets>

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Łączenie sygnałów i slotów z elementami UI
    connect(ui->pushButton_Close, SIGNAL(clicked()), this, SLOT(closeApp()));
//    createActions();
}

MainWindow::~MainWindow()
{
    delete ui;
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

// Łączenie sygnałów i slotów z elementami UI
void MainWindow::createActions()
{

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
