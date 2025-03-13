#include "test_widget.h"
#include "ui_test_widget.h"

Test_widget::Test_widget(Data_base* visual , QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Test_widget),
    visual_data(visual)
{
    ui->setupUi(this);
    appear_timer  = new QTimer();
    disappear_timer = new QTimer();
//    start_timer = new QTimer();
//    start_timer->setSingleShot(true);
//    connect(start_timer,&QTimer::timeout,this,&Test_widget::start_stimulate);
    connect(appear_timer,&QTimer::timeout,this,&Test_widget::appear_stimulate);
    connect(disappear_timer,&QTimer::timeout,this,&Test_widget::disappear_stimulate);
    Data_process  = new data_process();
    test_count.all_number=0;
    test_count.negative_test_number=0;
    test_count.positive_test_number=0;
    test_count.false_negative_number=0;
    test_count.false_positive_number=0;
//    initTestRegion(); // 初始化测试区域
}

Test_widget::~Test_widget()
{
    delete ui;
}

void Test_widget::start_stimulate()
{
    qDebug()<<"3s后即将开始测试，请做好准备";
    Test_flag =true;
    Appear_flag =true;
    QTimer::singleShot(3000, this, [this]() {
        appear_timer->start(200);
    });


}

void Test_widget::appear_stimulate()
{
    if(Appear_flag){
        appear_timer->stop();
        qDebug()<<"appear_stimulate";
        update();
        //避免update的异步性强制处理事件循环
        QCoreApplication::processEvents();
        Appear_flag = false;
        if(!uncheckedIndices.empty()) disappear_timer->start(200);
    }
}

void Test_widget::disappear_stimulate()
{
    if(!Appear_flag){
        disappear_timer->stop();
        qDebug()<<"disappear_stimulate";
        update();
        QCoreApplication::processEvents();
        Appear_flag = true;
        if(!uncheckedIndices.empty()) appear_timer->start(800);
    }
}

void Test_widget::Key_triger()
{


    if(now_check_point==nullptr) return;

    //判断有没有进行测试模组，与常规2选1
    if(test_check_point!=nullptr){

        if(test_check_point->in_negative){
            test_check_point->in_negative=false;
            qDebug()<<"假阴检测通过";
        }

        if(test_check_point->in_positive){
            test_check_point->in_positive=false;
            qDebug()<<"假阳检测通过";
        }

        test_check_point = nullptr;
        return;

    }

    if(now_check_point->check_pair == false){
        now_check_point->check_pair = true;
        Data_process->data_update(now_check_point);
        now_check_point->has_change = true;
    }



}

int Test_widget::caculate_index(RectROI *check_point)
{
    return check_point->row*9+check_point->col;
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
            roi.row = i;
            roi.col = j;
            roi.now_thresholds = (int(init_mean[i][j])-5)<0 ? 0 : int(init_mean[i][j])-5;
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
#if RESULTSEE
    for (int i = 0; i < Test_Region.size(); ++i) {
        if(Test_Region[i].blind_spot||Test_Region[i].invaild_area) continue;
        uncheckedIndices.insert(i);
    }
#endif


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
    //按照比例添加假阴假阳测试
    if(test_count.all_number%10==0){
        int randomChoice = std::rand() % 2;
        int test_index;

        for (int i = 0; i < Test_Region.size(); ++i) {
            if(Test_Region[i].blind_spot||Test_Region[i].invaild_area||Test_Region[i].early_thresholds==50) continue;
            testIndices.insert(i);
        }

        if(!testIndices.empty()){
            test_index = std::rand() % testIndices.size();
            auto it = testIndices.begin();
            std::advance(it, test_index);
            int selectedIndex = *it;
            test_check_point  = &Test_Region[selectedIndex];
            if (randomChoice == 0) {
                test_check_point->in_negative = true;
                test_count.negative_test_number++;
                qDebug()<<"假阴检测进行";
            } else {
                test_check_point->in_positive = true;
                test_count.positive_test_number++;
                qDebug()<<"假阳检测进行";
            }
            Data_process->data_update(test_check_point);
            QPoint showpoint = QPoint(start_x+now_check_point->col*rect_edge,start_x+now_check_point->row *rect_edge);
            int Gray = dBtoGray(test_check_point->test_show_thresholds);
            QColor grayColor(Gray, Gray, Gray);
            QPen pen(grayColor, 10, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            painter.setPen(pen);
            painter.drawPoint(showpoint);
            test_count.all_number++;
            return;
         }


    }
    //测试绘制，只存在一次




    //更新原先检测点的数据，若之前的测试没有反应，在这里清除
    if(test_check_point!=nullptr){
        if(test_check_point->in_positive) {
            test_count.false_positive_number++;
            test_check_point->in_positive = false;
            qDebug()<<"假阳检测未被反应";
        }
        else if(test_check_point->in_negative) {
            test_count.false_negative_number++;
            test_check_point->in_negative =false;
            qDebug()<<"假阴检测未被反应";
        }
        test_check_point = nullptr;
    }

    if(now_check_point!=nullptr) {
//        }
        Data_process->data_update(now_check_point);
        now_check_point->has_change = true;
        //检验now_check_point是否检测完成，若完成在uncheckedIndices去除
        if(now_check_point->check_over){
            int index = caculate_index(now_check_point);
            auto it = uncheckedIndices.find(index);
            if (it != uncheckedIndices.end()) {
                // 如果元素存在，则删除它
                uncheckedIndices.erase(it);
                qDebug() << "check region " << index << " has check over."<<"uncheckedIndices has : "<<uncheckedIndices.size();
            } else {
                qDebug()  << index << " not found in the set.";
            }
            if(uncheckedIndices.empty()) {
                qDebug()<< " uncheckedIndices no member test over";
                qDebug()<<"假阳测试："<<test_count.false_positive_number<<"/"<<test_count.positive_test_number;
                qDebug()<<"假阴测试："<<test_count.false_negative_number<<"/"<<test_count.negative_test_number;
                qDebug()<<"总共测试："<<test_count.all_number;
                return;
            }
        }
    }

    //随机选择待测点
    // 生成一个随机索引
    int randomIndex = std::rand() % uncheckedIndices.size();

    // 取第randomIndex个未检查区域的索引
    auto it = uncheckedIndices.begin();
    std::advance(it, randomIndex);
    int selectedIndex = *it;
    qDebug()<<"selectedIndex: "<< selectedIndex;
    //在测试区域产生刺激，绘制刺激点
    now_check_point = &Test_Region[selectedIndex];
    now_check_point->has_change = false;

    QPoint showpoint = QPoint(start_x+now_check_point->col*rect_edge,start_x+now_check_point->row *rect_edge);

    int Gray = dBtoGray(now_check_point->now_thresholds);
    qDebug()<<"now_thresholds: "<<now_check_point->now_thresholds <<" Gray: "<< Gray;
    QColor grayColor(Gray, Gray, Gray);
    QPen pen(grayColor, 10, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);
    painter.drawPoint(showpoint);

    emit change_display(Test_Region);

    test_count.all_number++;



}

void Test_widget::clear_background(QPainter &painter)
{
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::white);
    painter.drawRect(rect());

}

int Test_widget::dBtoGray(int dB)
{
    double exponent = dB / 10.0;
    double power = pow(10.0, exponent);

    // 计算L_T
    double L_T = 3184 / power + 10;

    double actural_screen_max = 300;
    double value = L_T/actural_screen_max;
    if (value < 0.0) {
        value = 0.0;
    } else if (value > 1.0) {
        value = 1.0;
    }

    // 线性映射
    int mappedValue = static_cast<int>(value * 255);

    return mappedValue;

}

void Test_widget::paintEvent(QPaintEvent *event)
{
    QImage image(size(), QImage::Format_ARGB32);
    image.fill(Qt::transparent);  // 填充透明背景

    // 创建 QPainter，用于在 QImage 上进行绘制
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    int back_color = static_cast<int>(1/30 * 255);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(back_color,back_color,back_color));
    painter.drawRect(rect());

    painter.setPen( QPen(Qt::red, 10, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    // 初始化画布
    if(!Test_flag) paint_init(painter);
    else {
        if(Appear_flag){
            //不断按次序刷新刺激点
            if (!uncheckedIndices.empty()){
                qDebug()<<"ready repaint";
                start_test(painter);
            }
            //待测试点集为空，停止刷新显示刺激定时器
            else{
                appear_timer->stop();
                disappear_timer->stop();
                Test_flag = true;
                Appear_flag = false;
                qDebug()<<"Test over";
                painter.setPen( QPen(Qt::red, 10, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                paint_init(painter);
            }
        }

    }
//    painter.drawPoint(size().width()/2,size().height()/2);
    // 将绘制的图像显示在窗口上
    painter.end();
    QPainter widgetPainter(this);
    widgetPainter.drawImage(0, 0, image);
    widgetPainter.setPen( QPen(Qt::red, 10, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    widgetPainter.drawPoint(width() / 2, height() / 2);
}

void Test_widget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Space:{
             qDebug()<<"响应";
             Key_triger();
             break;
        }
    case Qt::Key_Return:{
         qDebug()<<"开始测试";
         start_stimulate();
         break;
    }
    default:
        break;
    }
}


