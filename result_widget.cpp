#include "result_widget.h"
#include "ui_result_widget.h"

result_widget::result_widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::result_widget)
{
    ui->setupUi(this);
}

result_widget::~result_widget()
{
    delete ui;
}
