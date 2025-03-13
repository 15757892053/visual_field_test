#ifndef RESULT_WIDGET_H
#define RESULT_WIDGET_H

#include <QWidget>
#include <data/data.h>
#include <QPainter>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>

namespace Ui {
class result_widget;
}

class result_widget : public QWidget
{
    Q_OBJECT

public:
    explicit result_widget(Data_base* visual,QWidget *parent = nullptr);
    ~result_widget();
    void show_result(const std::vector<RectROI>& roi);

private:
    Ui::result_widget *ui;
    Data_base* visual_data;

    std::vector<std::tuple<int, int, float>> resize_thresholds(const std::vector<RectROI>& roi);


    void paint_Deviation_block(QPainter &painter , QRectF &rect ,double probability);
    int caculate_pattern(const std::vector<RectROI>& roi);

    void paint_thresholds_picture(const std::vector<RectROI>& roi);
    void paint_gray_picture(const std::vector<RectROI>& roi);
    void paint_TD_picture(const std::vector<RectROI>& roi);
    void paint_PD_picture(const std::vector<RectROI>& roi);
    void paint_TDP_picture(const std::vector<RectROI>& roi);
    void paint_PDP_picture(const std::vector<RectROI>& roi);


    double norm_cdf(double z) {
        return 0.5 * (1.0 + std::erf(z / std::sqrt(2.0)));  // 计算标准正态分布 CDF
    }


};

#endif // RESULT_WIDGET_H
