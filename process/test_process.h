#ifndef TEST_PROCESS_H
#define TEST_PROCESS_H
#include<data/data.h>

class data_process{
public:
    data_process();
    ~data_process();
    void data_update(RectROI* now_check_point,strategy Strategy = FAST);


private:
    void Fast_strategy(RectROI* now_check_point);

};





#endif TEST_PROCESS_H
