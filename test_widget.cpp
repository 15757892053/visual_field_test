#include "test_widget.h"
#include "ui_test_widget.h"

Test_widget::Test_widget(Data_base* visual , QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Test_widget),
    visual_data(visual)
{
    ui->setupUi(this);
//    initTestRegion(); // 初始化测试区域
}

Test_widget::~Test_widget()
{
    delete ui;
}

void Test_widget::appear_stimulate()
{

}

void Test_widget::disappear_stimulate()
{

}

void Test_widget::initTestRegion()
{
    eye_type eye = visual_data->get_eyetype();
    QVector<QVector<double>> init_mean = visual_data->Get_mean_threshold();
    //如果是左眼则将初始阈值数据进行水平反转，因为存储都是按照右眼格式进行存储
    if(eye == LEFT_EYE){
        for (auto& row : init_mean) {
            int left = 0;
            int right = row.size() - 1;
            while (left < right) {
                double temp = row[left];
                row[left] = row[right];
                row[right] = temp;
                left++;
                right--;
            }
        }
    }


    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 9; ++j) {
            RectROI roi;
            roi.row = j;
            roi.col = i;
            roi.now_thresholds = int(init_mean[i][j]);
            if(isInTopLeftTriangle(j, i,eye) ||
               isInBottomLeftTriangle(j, i,eye) ||
               isInTopRightTriangle(j, i,eye) ||
               isInBottomRightTriangle(j, i,eye)){
                roi.invaild_area = true;
            }
            if(isAtSpecificPoint(j,i,eye)){
                roi.blind_spot=true;
            }


            Test_Region.push_back(roi); // 假设 RectROI 有 col 和 row 成员


        }
    }
    if(eye == LEFT_EYE){
        start_x = size().width() / 2 - 4.5 * rect_edge; // 计算起始 x 坐标
    }
    else start_x = size().width() / 2 - 3.5 * rect_edge; // 计算起始 x 坐标

    start_y = size().height() / 2 - 3.5 * rect_edge; // 计算起始 y 坐标
    //将要检测的区域加入到set中
    for (int i = 0; i < Test_Region.size(); ++i) {
        if(Test_Region[i].blind_spot||Test_Region[i].invaild_area) continue;
        uncheckedIndices.insert(i);
    }

}

void Test_widget::paint_init(QPainter &painter)
{
    // 初始化 9x8 的点阵
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 9; ++j) {
            int x = start_x + j * rect_edge; // 计算当前点的 x 坐标
            int y = start_y + i * rect_edge; // 计算当前点的 y 坐标
            points.push_back(QPoint(x, y)); // 存储点的坐标
        }
    }

    // 绘制点阵
    for (const auto &point : points) {
        painter.drawPoint(point); // 绘制点
    }
}

void Test_widget::start_test(QPainter &painter)
{
    //随机选择待测点
    // 生成一个随机索引
    int randomIndex = std::rand() % uncheckedIndices.size();

    // 取第randomIndex个未检查区域的索引
    auto it = uncheckedIndices.begin();
    std::advance(it, randomIndex);
    int selectedIndex = *it;

    //在测试区域产生刺激，绘制刺激点
    now_check_point = &Test_Region[selectedIndex];
    QPoint showpoint = QPoint(start_x+now_check_point->row*rect_edge,start_x+now_check_point->col*rect_edge);

    // 确保插值结果在 0 到 30 之间
    int interpolatedValue = std::max(0, std::min(30, now_check_point->now_thresholds));
    // 线性映射到灰度值
    float normalized = interpolatedValue / 30.0f;
    float sigmoid = 1.0f / (1.0f + std::exp(-10 * (normalized - 0.5f)));
    int Gray = static_cast<int>(sigmoid * 255.0f);

    QColor grayColor(Gray, Gray, Gray);
    QPen pen(grayColor, 10, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);
    painter.drawPoint(showpoint);
    //更新区域参数，修改校验对，用于验证假阴假阳


}

void Test_widget::clear_background(QPainter &painter)
{
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::white);
    painter.drawRect(rect());

}

void Test_widget::paintEvent(QPaintEvent *event)
{
    QImage image(size(), QImage::Format_ARGB32);
    image.fill(Qt::transparent);  // 填充透明背景

    // 创建 QPainter，用于在 QImage 上进行绘制
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::white);
    painter.drawRect(rect());

    painter.setPen( QPen(Qt::red, 10, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    // 初始化画布
    if(!Test_flag) paint_init(painter);
    else {
        if(Appear_flag){
            //不断按次序刷新刺激点
            if (!uncheckedIndices.empty()){
                start_test(painter);
            }
            //待测试点集为空，停止刷新显示刺激定时器
            else{
                return;
            }
        }

    }
    painter.drawPoint(size().width()/2,size().height()/2);
    // 将绘制的图像显示在窗口上
    painter.end();
    QPainter widgetPainter(this);
    widgetPainter.drawImage(0, 0, image);
}


