#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    viusal_database = new Data_base();
    viusal_database->init_database();
    test_widget = new Test_widget(viusal_database);
    // 创建独立的 QSqlTableModel 对象
    QSqlTableModel* personModel = new QSqlTableModel(this, viusal_database->get_database());
    personModel->setTable("new_mean_data_L");
    personModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    if (!personModel->select()) {
        QMessageBox::critical(this, "错误信息", "打开 person 表错误,错误信息:\n" + personModel->lastError().text());
        return;
    }

    QSqlTableModel* eyeModel = new QSqlTableModel(this, viusal_database->get_database());
    eyeModel->setTable("new_mean_data_R");
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

    bool regis_flag = people_message_register();
    if(!regis_flag) return;
    test_widget->initTestRegion();
    test_widget->show();
    this->hide();

}





void MainWindow::on_database_change_clicked()
{
    stack_index = (++stack_index) % 3;
    this->ui->stackedWidget->setCurrentIndex(stack_index);
}

bool MainWindow::people_message_register()
{
    if(this->ui->Left_eye_checkBox->isChecked())        viusal_database->set_eyetype(LEFT_EYE);
    else if(this->ui->Left_eye_checkBox->isChecked())   viusal_database->set_eyetype(RIGHT_EYE);
    else {
        qWarning()<<"please choose test eye";
        return false;
    }

    if(this->ui->man_checkBox->isChecked())  viusal_database->set_gender(MAN);
    else if(this->ui->female_checkBox->isChecked()) viusal_database->set_gender(FEMALE);
    else {
        qWarning()<<"please choose your Gender";
        return false;
    }

    QString ageText = this->ui->age_lineEdit->text();

    bool ok;
    int age = ageText.toInt(&ok);
    if (!ok) {
        // 输入不是有效的整数
        QMessageBox::critical(this, "错误", "输入的年龄不是有效的整数，请重新输入。");
    }
    else viusal_database->set_age(age);

    return true;


}

