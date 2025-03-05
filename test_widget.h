#ifndef TEST_WIDGET_H
#define TEST_WIDGET_H

#include <QWidget>
#include <cmath>
#include <vector>
#include <data/data.h>
#include <QPainter>
#include <unordered_set>
#include <windows.h>
#include <QKeyEvent>
#include <process/test_process.h>
namespace Ui {
class Test_widget;
}

//视野检测的图像显示界面
//1.提示用户注意事项，待其准备完毕后按键确认
class Test_widget : public QWidget
{
    Q_OBJECT

public:
    explicit Test_widget(Data_base* visual,QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void initTestRegion(); // 初始化测试区域以及阈值起点
    std::vector<RectROI> Get_ROI(){return Test_Region;}
    ~Test_widget();

signals :
    void change_display(std::vector<RectROI>& roi);


private slots:
    void start_stimulate();

    void appear_stimulate();

    void disappear_stimulate();



private:
    Ui::Test_widget *ui;
    bool Test_flag=false;
    bool Appear_flag = false;
    int triger = 0;
    QTimer* appear_timer;
    QTimer* disappear_timer;

    int rect_edge = 50;
    int start_x; // 起始 x 坐标
    int start_y; // 起始 y 坐标
    int test_time = 0;
    RectROI* now_check_point = nullptr;
    Data_base* visual_data;
    data_process* Data_process;
    std::vector<QPoint> points; // 存储点的坐标
    std::vector<RectROI> Test_Region; // 测试区域
    std::unordered_set<int>  uncheckedIndices;

    //按键响应
    void Key_triger();

    //辅助函数
    int caculate_index(RectROI* now_check_point);
    void paint_init(QPainter &painter); // 绘制初始化
    void start_test(QPainter &painter);
    void clear_background(QPainter &painter);
    void set_background(int light);
    int  dBtoGray(int dB);


    // 检查点是否在左上边长为 1 的正三角形区域
    static bool isInTopLeftTriangle(int x, int y ,eye_type eye) {
        return eye==LEFT_EYE? (x == 0 && y == 0) || (x == 0 && y == 1) || (x == 1 && y == 0):
                   (x == 0 && (y == 0 || y == 1 || y == 2)) ||(x == 1 && (y == 0 || y == 1)) ||(x == 2 && y == 0);
    }

    // 检查点是否在左下边长为 1 的正三角形区域
    static bool isInBottomLeftTriangle(int x, int y,eye_type eye) {
        return eye==LEFT_EYE? (x == 0 && y == 6) || (x == 0 && y == 7) || (x == 1 && y == 7):
                   (x == 0 && (y == 5 || y == 6 || y == 7)) ||(x == 1 && (y == 6 || y == 7)) ||(x == 2 && y == 7);
    }

    // 检查点是否在右上边长为 2 的正三角形区域
    static bool isInTopRightTriangle(int x, int y,eye_type eye) {
        return eye==LEFT_EYE? (x >= 6 && y == 0) || (x >= 7 && y == 1) || (x >= 8 && y == 2):
                   (x == 8 && y == 0) || (x == 7 && y == 0) || (x == 8 && y == 1);
    }

    // 检查点是否在右下边长为 2 的正三角形区域
    static bool isInBottomRightTriangle(int x, int y,eye_type eye) {
        return eye==LEFT_EYE? (x >= 8 && y == 5) || (x >= 7 && y == 6) || (x >= 6 && y == 7):
                   (x == 8 && y == 7) || (x == 7 && y == 7) || (x == 8 && y == 6);
    }

    // 检查点是否为指定坐标 (4, 1)
    static bool isAtSpecificPoint(int x, int y,eye_type eye) {
        return eye==LEFT_EYE? x == 1 && y == 4:x == 7 && y == 4;
    }





};

#endif // TEST_WIDGET_H
