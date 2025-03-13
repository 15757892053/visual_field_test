#include "result_widget.h"
#include "ui_result_widget.h"

result_widget::result_widget(Data_base *visual, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::result_widget),
    visual_data(visual)
{
    ui->setupUi(this);
}

result_widget::~result_widget()
{
    delete ui;
}

void result_widget::show_result(const std::vector<RectROI> &roi)
{

    paint_thresholds_picture(roi);
    paint_TD_picture(roi);
    paint_gray_picture(roi);
    paint_PD_picture(roi);
    paint_TDP_picture(roi);
    paint_PDP_picture(roi);

}

std::vector<std::tuple<int, int, float> > result_widget::resize_thresholds(const std::vector<RectROI> &roi)
{
    int width = 18;
    int height = 16;
    std::vector<std::tuple<int, int, float> > result;
    // 创建空白灰度图像
    cv::Mat grayImage(height, width, CV_8UC1, cv::Scalar(0));
    float epsilon =0.6f;
    //    qDebug()<< epsilon;

    // 调节参数 epsilon 和 lambda
    float lambda = 0.1;

    int n = roi.size();

    // 构建Phi矩阵
    cv::Mat Phi(n, n, CV_32F);

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            float r = distance(roi[i].col, roi[i].row, roi[j].col, roi[j].row);
            // 使用改进的核函数
            Phi.at<float>(i, j) = std::exp(-epsilon * r * r);
        }
    }

    // 添加正则化项
    cv::Mat I = cv::Mat::eye(n, n, CV_32F);
    Phi += lambda * I;

    // 构建值向量
    cv::Mat value(n, 1, CV_32F);
    for (int i = 0; i < n; i++) {
        value.at<float>(i, 0) = roi[i].now_thresholds;
    }

    // 求解权重向量w
    cv::Mat w;
    cv::solve(Phi, value, w, cv::DECOMP_SVD);

    // 映射灰度值并生成图像
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float mappedX = (x / static_cast<float>(width-1)) * 8.0f;
            float mappedY = (y / static_cast<float>(height-1)) * 7.0f;

            float interpolatedValue = 0.0f;
            for (int i = 0; i < n; i++) {
                float r = distance(mappedX, mappedY, roi[i].col, roi[i].row);
                float phi = std::exp(-epsilon * r * r);
                interpolatedValue += w.at<float>(i, 0) * phi;
            }

            // 确保插值结果在 0 到 30 之间
            interpolatedValue = std::max(0.0f, std::min(35.0f, interpolatedValue));
            //            outfile << x << "," << y << "," << interpolatedValue << std::endl;
            result.push_back(std::make_tuple(x, y, interpolatedValue));
        }
    }


    return result;
}

void result_widget::paint_Deviation_block(QPainter &painter, QRectF &rect, double probability)
{
    int delta = rect.width()/10;
    int centerX = rect.width()/2+rect.x();
    int centerY = rect.height() / 2+rect.y();
    QRectF small_rect(rect.x()+2*delta,rect.y()+delta,rect.width()-3*delta,rect.height()-2*delta);
    QBrush brush;
    if(probability<0.005) {
      brush = QBrush(Qt::black,Qt::SolidPattern);
      painter.fillRect(small_rect, brush);
    }
    else if(probability<0.01) {
      brush = QBrush(Qt::black,Qt::Dense2Pattern) ;
      painter.fillRect(small_rect, brush);
    }
    else if(probability<0.02) {
      brush = QBrush(Qt::black,Qt::Dense6Pattern);
      painter.fillRect(small_rect, brush);
    }
    else if (probability<0.05){
        painter.setPen(QPen(Qt::black, 3));
        painter.drawPoint(small_rect.x()+3*delta, small_rect.y()+3*delta);
        painter.drawPoint(small_rect.x()+6*delta, small_rect.y()+3*delta);
        painter.drawPoint(small_rect.x()+3*delta, small_rect.y()+6*delta);
        painter.drawPoint(small_rect.x()+6*delta, small_rect.y()+6*delta);
    }
    else{
        painter.setPen(QPen(Qt::black, 3));
        painter.drawPoint(centerX, centerY);
    }

}

int result_widget::caculate_pattern(const std::vector<RectROI> &roi)
{
    std::vector<int> td;
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

    for(auto test_roi : roi){
        if(test_roi.blind_spot||test_roi.invaild_area) continue;
        double td_double = test_roi.now_thresholds - init_mean[test_roi.row][test_roi.col];
        int td_int = static_cast<int>(td_double);  // 直接转换为整数
        td.push_back(td_int);
    }

    // 如果没有有效数据，返回 0
    if (td.empty()) return 0;

    // 计算 85% 分位数
    size_t percentile_index = static_cast<size_t>(std::ceil(td.size() * 0.85)) - 1;
    percentile_index = std::min(percentile_index, td.size() - 1);  // 防止越界

    std::nth_element(td.begin(), td.begin() + percentile_index, td.end());
    int percentile_85 = td[percentile_index];

    return percentile_85;
}

void result_widget::paint_thresholds_picture(const std::vector<RectROI> &roi)
{

    int width = this->ui->thresholds_label->width(), height = this->ui->thresholds_label->height();
    QImage image(width, height, QImage::Format_Grayscale8);
    image.fill(Qt::white);  // 先填充白色背景

    QPainter painter(&image);
    painter.setPen(QPen(Qt::black, 3));  // 设置画笔颜色和线宽
    // 计算中心点
    int centerX = width / 2;
    int centerY = height / 2;

    // 画十字线
    painter.drawLine(centerX, 0, centerX, height);  // 垂直线
    painter.drawLine(0, centerY, width, centerY);  // 水平线
//    painter.end();  // 结束绘制

    //画视角标线
    // 视角范围（假设最大视角对应 QLabel 的 90% 宽度）
    int maxRadius = std::min(width, height);  // 30° 视角的像素半径
    int stepRadius = maxRadius / 6;  // 每 10° 视角的半径增量

    // 画 10°、20°、30° 视角标线
    for (int i = 1; i <= 3; ++i) {
        int radius = i * stepRadius;  // 当前 10° 视角的像素半径
        int tickSize = 10;  // 标线的长度（像素）

        // 在上下左右四个方向画标线
        painter.drawLine(centerX - tickSize / 2, centerY - radius, centerX + tickSize / 2, centerY - radius);  // 上
        painter.drawLine(centerX - tickSize / 2, centerY + radius, centerX + tickSize / 2, centerY + radius);  // 下
        painter.drawLine(centerX - radius, centerY - tickSize / 2, centerX - radius, centerY + tickSize / 2);  // 左
        painter.drawLine(centerX + radius, centerY - tickSize / 2, centerX + radius, centerY + tickSize / 2);  // 右
    }


    //绘制阈值
    int rectWidth = width / 10;  // 每个矩形宽度
    int rectHeight = height / 10;  // 每个矩形高度


    //判断左眼还是右眼
    if(visual_data->get_eyetype()==LEFT_EYE){
        for(auto test_roi : roi){
            if(test_roi.blind_spot||test_roi.invaild_area) continue;
            int x = (test_roi.col+1)*rectWidth;
            int y = (test_roi.row+1)*rectHeight;
            QString text = QString::number(test_roi.now_thresholds, 'f', 0);
            QRectF rect(x, y, rectWidth,rectHeight);
            painter.drawText(rect, Qt::AlignCenter, text);

        }

    }
    else if(visual_data->get_eyetype() == RIGHT_EYE){
        for(auto test_roi : roi){
            if(test_roi.blind_spot||test_roi.invaild_area) continue;
            int x = test_roi.col*rectWidth;
            int y = (test_roi.row+1)*rectHeight;
            QString text = QString::number(test_roi.now_thresholds, 'f', 0);
            QRectF rect(x, y, rectWidth,rectHeight);
            painter.drawText(rect, Qt::AlignCenter, text);
        }
    }



    this->ui->thresholds_label->setPixmap(QPixmap::fromImage(image));
    this->ui->thresholds_label->setScaledContents(true);

}

void result_widget::paint_gray_picture(const std::vector<RectROI> &roi)
{

    std::vector<std::tuple<int, int, float>> re_threshold = resize_thresholds(roi);

    int width = this->ui->thresholds_label->width(), height = this->ui->thresholds_label->height();
    QImage image(width, height, QImage::Format_Grayscale8);
    image.fill(Qt::white);  // 先填充白色背景

    QPainter painter(&image);
    painter.setPen(QPen(Qt::black, 3));  // 设置画笔颜色和线宽
    // 计算中心点
    int centerX = width / 2;
    int centerY = height / 2;

    // 画十字线
    painter.drawLine(centerX, 0, centerX, height);  // 垂直线
    painter.drawLine(0, centerY, width, centerY);  // 水平线

    int rectWidth = width / 20;  // 每个矩形宽度
    int rectHeight = height / 20;  // 每个矩形高度
    auto eye = visual_data->get_eyetype();
    //判断左眼还是右眼
    if(eye==LEFT_EYE){
        for(auto test_roi : re_threshold){
            int i = std::get<0>(test_roi);
            int j = std::get<1>(test_roi);

            if(isInTopLeftTriangle(i/2, j/2,eye) ||
                isInBottomLeftTriangle(i/2, j/2,eye) ||
                isInTopRightTriangle(i/2, j/2,eye) ||
                isInBottomRightTriangle(i/2, j/2,eye)){
                continue;
            }

            int x = (i+2)*rectWidth;
            int y = (j+2)*rectHeight;
            QRectF rect(x, y, rectWidth, rectHeight);

            // 计算灰度等级 (5 级)
            double threshold = std::get<2>(test_roi);
            QBrush brush;
            if (threshold < 5) brush = QBrush(Qt::black,Qt::Dense2Pattern); // 全黑
            else if (threshold < 10) brush = QBrush(Qt::gray, Qt::Dense3Pattern); // 密集灰
            else if (threshold < 15) brush = QBrush(Qt::gray, Qt::Dense5Pattern); // 普通灰
            else if (threshold < 20) brush = QBrush(Qt::gray, Qt::Dense6Pattern); // 小三角灰
            else  brush = QBrush(Qt::gray, Qt::Dense7Pattern); // 16 个灰点

            painter.fillRect(rect, brush);
        }

    }
    else if(eye == RIGHT_EYE){
        for(auto test_roi : re_threshold){
            int i = std::get<0>(test_roi);
            int j = std::get<1>(test_roi);
            if(isInTopLeftTriangle(i/2, j/2,eye) ||
                isInBottomLeftTriangle(i/2, j/2,eye) ||
                isInTopRightTriangle(i/2, j/2,eye) ||
                isInBottomRightTriangle(i/2, j/2,eye)){
                continue;
            }

            int x = (i)*rectWidth;
            int y = (j+2)*rectHeight;
            QRectF rect(x, y, rectWidth, rectHeight);

            // 计算灰度等级 (5 级)
            double threshold = std::get<2>(test_roi);
            QBrush brush;
            if (threshold < 5) brush = QBrush(Qt::black,Qt::Dense2Pattern); // 全黑
            else if (threshold < 10) brush = QBrush(Qt::gray, Qt::Dense3Pattern); // 密集灰
            else if (threshold < 15) brush = QBrush(Qt::gray, Qt::Dense5Pattern); // 普通灰
            else if (threshold < 20) brush = QBrush(Qt::gray, Qt::Dense6Pattern); // 小三角灰
            else  brush = QBrush(Qt::gray, Qt::Dense7Pattern); // 16 个灰点
            painter.fillRect(rect, brush);
        }
    }

    this->ui->gray_label->setPixmap(QPixmap::fromImage(image));
    this->ui->gray_label->setScaledContents(true);



}

void result_widget::paint_TD_picture(const std::vector<RectROI> &roi)
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


    int width = this->ui->thresholds_label->width(), height = this->ui->thresholds_label->height();
    QImage image(width, height, QImage::Format_Grayscale8);
    image.fill(Qt::white);  // 先填充白色背景

    QPainter painter(&image);
    painter.setPen(QPen(Qt::black, 3));  // 设置画笔颜色和线宽
    // 计算中心点
    int centerX = width / 2;
    int centerY = height / 2;

    // 画十字线
    painter.drawLine(centerX, 0, centerX, height);  // 垂直线
    painter.drawLine(0, centerY, width, centerY);  // 水平线


    //绘制阈值
    int rectWidth = width / 10;  // 每个矩形宽度
    int rectHeight = height / 10;  // 每个矩形高度


    //判断左眼还是右眼
    if(visual_data->get_eyetype()==LEFT_EYE){
        for(auto test_roi : roi){
            if(test_roi.blind_spot||test_roi.invaild_area) continue;
            int x = (test_roi.col+1)*rectWidth;
            int y = (test_roi.row+1)*rectHeight;
            double td = test_roi.now_thresholds - init_mean[test_roi.row][test_roi.col];
            int td_int = static_cast<int>(td);  // 直接转换为整数
            QString text = QString::number(td_int);  // 转换为字符串
            QRectF rect(x, y, rectWidth, rectHeight);
            painter.drawText(rect, Qt::AlignCenter, text);
        }
    }
    else if(visual_data->get_eyetype() == RIGHT_EYE){
        for(auto test_roi : roi){
            if(test_roi.blind_spot||test_roi.invaild_area) continue;
            int x = test_roi.col*rectWidth;
            int y = (test_roi.row+1)*rectHeight;
            double td = test_roi.now_thresholds - init_mean[test_roi.row][test_roi.col];
            int td_int = static_cast<int>(td);  // 直接转换为整数
            QString text = QString::number(td_int);  // 转换为字符串

            QRectF rect(x, y, rectWidth, rectHeight);
            painter.drawText(rect, Qt::AlignCenter, text);
        }
    }


    this->ui->TD_label->setPixmap(QPixmap::fromImage(image));
    this->ui->TD_label->setScaledContents(true);

}

void result_widget::paint_PD_picture(const std::vector<RectROI> &roi)
{

    int width = this->ui->thresholds_label->width(), height = this->ui->thresholds_label->height();
    QImage image(width, height, QImage::Format_Grayscale8);
    image.fill(Qt::white);  // 先填充白色背景

    QPainter painter(&image);
    painter.setPen(QPen(Qt::black, 3));  // 设置画笔颜色和线宽
    // 计算中心点
    int centerX = width / 2;
    int centerY = height / 2;

    // 画十字线
    painter.drawLine(centerX, 0, centerX, height);  // 垂直线
    painter.drawLine(0, centerY, width, centerY);  // 水平线

    int rectWidth = width / 10;  // 每个矩形宽度
    int rectHeight = height / 10;  // 每个矩形高度

    //判断左眼还是右眼
    for(auto test_roi : roi){
        if(test_roi.blind_spot||test_roi.invaild_area) continue;
        int x = visual_data->get_eyetype()==LEFT_EYE ? (test_roi.col+1)*rectWidth : (test_roi.col)*rectWidth;
        int y = (test_roi.row+1)*rectHeight;

        QRectF rect(x, y, rectWidth,rectHeight);
        auto mean_std = visual_data->Get_CDF(test_roi.col,test_roi.row);
        double threshold = test_roi.now_thresholds;
        qDebug()<<"("<<test_roi.col<<","<<test_roi.row<<")"<<" mean:"<< mean_std.first<<"now_thresholds: "<<threshold;
        // 计算 Z-score
        double Z = (mean_std.second > 0) ? (threshold - mean_std.first) / mean_std.second : 0;
        double TDP = norm_cdf(Z);
        qDebug()<<"("<<test_roi.col<<","<<test_roi.row<<")"<<" TDP:"<< TDP;
        paint_Deviation_block(painter,rect,TDP);

    }

    this->ui->td_dev_label_3->setPixmap(QPixmap::fromImage(image));
    this->ui->td_dev_label_3->setScaledContents(true);


}

void result_widget::paint_TDP_picture(const std::vector<RectROI> &roi)
{
    int delta = caculate_pattern(roi);
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


    int width = this->ui->thresholds_label->width(), height = this->ui->thresholds_label->height();
    QImage image(width, height, QImage::Format_Grayscale8);
    image.fill(Qt::white);  // 先填充白色背景

    QPainter painter(&image);
    painter.setPen(QPen(Qt::black, 3));  // 设置画笔颜色和线宽
    // 计算中心点
    int centerX = width / 2;
    int centerY = height / 2;

    // 画十字线
    painter.drawLine(centerX, 0, centerX, height);  // 垂直线
    painter.drawLine(0, centerY, width, centerY);  // 水平线


    //绘制阈值
    int rectWidth = width / 10;  // 每个矩形宽度
    int rectHeight = height / 10;  // 每个矩形高度



    for(auto test_roi : roi){
        if(test_roi.blind_spot||test_roi.invaild_area) continue;
        int x = visual_data->get_eyetype()==LEFT_EYE?(test_roi.col+1)*rectWidth:(test_roi.col)*rectWidth;
        int y = (test_roi.row+1)*rectHeight;
        double td = test_roi.now_thresholds - init_mean[test_roi.row][test_roi.col]-delta;
        int td_int = static_cast<int>(td);  // 直接转换为整数
        QString text = QString::number(td_int);  // 转换为字符串
        QRectF rect(x, y, rectWidth, rectHeight);
        painter.drawText(rect, Qt::AlignCenter, text);
    }




    this->ui->pattern_td_label_2->setPixmap(QPixmap::fromImage(image));
    this->ui->pattern_td_label_2->setScaledContents(true);



}

void result_widget::paint_PDP_picture(const std::vector<RectROI> &roi)
{
    int delta = caculate_pattern(roi);
    int width = this->ui->thresholds_label->width(), height = this->ui->thresholds_label->height();
    QImage image(width, height, QImage::Format_Grayscale8);
    image.fill(Qt::white);  // 先填充白色背景

    QPainter painter(&image);
    painter.setPen(QPen(Qt::black, 3));  // 设置画笔颜色和线宽
    // 计算中心点
    int centerX = width / 2;
    int centerY = height / 2;

    // 画十字线
    painter.drawLine(centerX, 0, centerX, height);  // 垂直线
    painter.drawLine(0, centerY, width, centerY);  // 水平线

    int rectWidth = width / 10;  // 每个矩形宽度
    int rectHeight = height / 10;  // 每个矩形高度

    //判断左眼还是右眼
    for(auto test_roi : roi){
        if(test_roi.blind_spot||test_roi.invaild_area) continue;
        int x = visual_data->get_eyetype()==LEFT_EYE ? (test_roi.col+1)*rectWidth : (test_roi.col)*rectWidth;
        int y = (test_roi.row+1)*rectHeight;

        QRectF rect(x, y, rectWidth,rectHeight);
        auto mean_std = visual_data->Get_CDF(test_roi.col,test_roi.row);
        double threshold = test_roi.now_thresholds;
        qDebug()<<"("<<test_roi.col<<","<<test_roi.row<<")"<<" mean:"<< mean_std.first<<"now_thresholds: "<<threshold;
        // 计算 Z-score
        double Z = (mean_std.second > 0) ? (threshold - mean_std.first-delta) / mean_std.second : 0;
        double TDP = norm_cdf(Z);
        qDebug()<<"("<<test_roi.col<<","<<test_roi.row<<")"<<" TDP:"<< TDP;
        paint_Deviation_block(painter,rect,TDP);

    }

    this->ui->pattern_td_dev_label_2->setPixmap(QPixmap::fromImage(image));
    this->ui->pattern_td_dev_label_2->setScaledContents(true);

}
