#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <test_widget.h>
#include <result_widget.h>
#include <data/data.h>
#include <QMessageBox>
#include <../creat_losspic/discheckpoint.h>
#include <vector>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void display_data(std::vector<RectROI>& roi);

private slots:
    void on_pushButton_clicked();

    void on_database_change_clicked();

    void on_pushButton_2_clicked();

private:
    int stack_index = 0;
    Ui::MainWindow *ui;
    Test_widget* test_widget;
    result_widget* Result_widget;
    DisCheckPoint* dis_widget;
    Data_base* viusal_database;

    bool people_message_register();

};
#endif // MAINWINDOW_H
