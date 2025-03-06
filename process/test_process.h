#ifndef TEST_PROCESS_H
#define TEST_PROCESS_H
#include<data/data.h>

class data_process{
public:
    data_process();
    ~data_process();
    void data_update(RectROI* now_check_point,strategy Strategy = FAST);


private:

    int test_time = 0;
    int false_positive_time = 0;
    int false_nagetive_time = 0;

    void Fast_strategy(RectROI* now_check_point);
    //假阴性测试，原本看不见的区域，测出来为看的见 -->反应造假可能性
    void False_negative_tests(RectROI* now_check_point);
    //假阳性测试，原本看的见的区域，没有反应过-->反应专注度
    void False_positive_tests(RectROI* now_check_point);
};





#endif TEST_PROCESS_H
