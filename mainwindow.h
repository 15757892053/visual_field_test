#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <test_widget.h>
#include <data/data.h>
#include <QMessageBox>

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
    void on_pushButton_clicked();

    void on_database_check_clicked();

    void on_database_change_clicked();

private:
    int stack_index = 0;
    Ui::MainWindow *ui;
    Test_widget* test_widget;
    Data_base* viusal_database;

};
#endif // MAINWINDOW_H
