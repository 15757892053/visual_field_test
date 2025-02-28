#ifndef RESULT_WIDGET_H
#define RESULT_WIDGET_H

#include <QWidget>

namespace Ui {
class result_widget;
}

class result_widget : public QWidget
{
    Q_OBJECT

public:
    explicit result_widget(QWidget *parent = nullptr);
    ~result_widget();

private:
    Ui::result_widget *ui;
};

#endif // RESULT_WIDGET_H
