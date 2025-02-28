#ifndef TEST_WIDGET_H
#define TEST_WIDGET_H

#include <QWidget>
#include <vector>
#include <data/data.h>
#include <QPainter>
#include <unordered_set>

namespace Ui {
class Test_widget;
}

//视野检测的图像显示界面
//1.提示用户注意事项，待其准备完毕后按键确认
//2.
class Test_widget : public QWidget
{
    Q_OBJECT

public:
    explicit Test_widget(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *event) override;
    ~Test_widget();

private:
    Ui::Test_widget *ui;
    bool Test_flag=false;
    bool Appear_flag = false;
    int triger = 0;
    QTimer* appear_timer;
    int rect_edge = 50;
    int start_x; // 起始 x 坐标
    int start_y; // 起始 y 坐标
    RectROI* now_check_point;
    std::vector<QPoint> points; // 存储点的坐标
    std::vector<RectROI> Test_Region; // 测试区域
    std::unordered_set<int>  uncheckedIndices;

    //辅助函数
    void initTestRegion(); // 初始化测试区域以及阈值起点
    void paint_init(QPainter &painter); // 绘制初始化
    void start_test(QPainter &painter);
    void clear_background(QPainter &painter);
    void set_background(int light);





};

#endif // TEST_WIDGET_H
