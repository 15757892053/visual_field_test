#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    test_widget = new Test_widget();
    viusal_database = new Data_base();
    viusal_database->init_database();
    // 创建独立的 QSqlTableModel 对象
    QSqlTableModel* personModel = new QSqlTableModel(this, viusal_database->get_database());
    personModel->setTable("mean_data_L");
    personModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    if (!personModel->select()) {
        QMessageBox::critical(this, "错误信息", "打开 person 表错误,错误信息:\n" + personModel->lastError().text());
        return;
    }

    QSqlTableModel* eyeModel = new QSqlTableModel(this, viusal_database->get_database());
    eyeModel->setTable("mean_data_R");
    eyeModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    if (!eyeModel->select()) {
        QMessageBox::critical(this, "错误信息", "打开 eye 表错误,错误信息:\n" + eyeModel->lastError().text());
        return;
    }

    QSqlTableModel* testDataModel = new QSqlTableModel(this, viusal_database->get_database());
    testDataModel->setTable("test_data");
    testDataModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    if (!testDataModel->select()) {
        QMessageBox::critical(this, "错误信息", "打开 test_data 表错误,错误信息:\n" + testDataModel->lastError().text());
        return;
    }

    // 绑定模型到 QTableView
    this->ui->stackedWidget->setCurrentIndex(0);
    QTableView* tableView1 = this->ui->stackedWidget->currentWidget()->findChild<QTableView*>("tableView");
    tableView1->setModel(personModel);

    this->ui->stackedWidget->setCurrentIndex(1);
    QTableView* tableView2 = this->ui->stackedWidget->currentWidget()->findChild<QTableView*>("tableView_2");
    tableView2->setModel(eyeModel);

    this->ui->stackedWidget->setCurrentIndex(2);
    QTableView* tableView3 = this->ui->stackedWidget->currentWidget()->findChild<QTableView*>("tableView_3");
    tableView3->setModel(testDataModel);

    // 设置初始页面
    this->ui->stackedWidget->setCurrentIndex(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    test_widget->show();

    this->hide();

}





void MainWindow::on_database_change_clicked()
{
    stack_index = (++stack_index) % 3;
    this->ui->stackedWidget->setCurrentIndex(stack_index);
}

