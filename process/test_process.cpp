#include <process/test_process.h>

void data_process::data_update(RectROI *now_check_point, strategy Strategy)
{
    //假阴假阳测试占一定比例

    switch (Strategy) {
    case FAST:
        Fast_strategy(now_check_point);
        break;
    default:
        break;
    }

}

void data_process::Fast_strategy(RectROI *now_check_point)
{

    //如果之前已经修改过来就不用再次修改
    if(now_check_point->has_change) return;
    //始终是在看不见的时候确定阈值-->下降的时候

    //处理假阴假阳测试
    if(now_check_point->in_negative){
        False_negative_tests(now_check_point);
    }

    //case 1 : 按键确认，下降阈值
    if(now_check_point->check_pair){
        //判断是不是转折点，即由上升到下降
        if(now_check_point->direact == UP){
            //转折需求数量-1
            now_check_point->change_num--;
            now_check_point->direact = DOWN;
            now_check_point->change_value = -now_check_point->change_value-1;
            now_check_point->nonesee = true;
            now_check_point->need_count = 1;
        }

        now_check_point->early_thresholds = now_check_point->now_thresholds;
        now_check_point->now_thresholds = now_check_point->now_thresholds+now_check_point->change_value;
        now_check_point->check_pair = false;
    }
    //case 2:向下转折后首次没有按键反应，重复几次都看不见就开始上升阈值
    else{
        //need_count数量-1，代表还需要几次没有反应才说明看不见
        if(now_check_point->nonesee && now_check_point->need_count!=0){
            now_check_point->need_count--;
            qDebug()<<"(x,y): ("<<now_check_point->col<<","<<now_check_point->row<<"),点未被反应,当前还需要不被注意到："<<now_check_point->need_count<<"次";
            return;
        }
        now_check_point->nonesee = false;

        //如果若转折次数已经达到要求，确认该点检测完毕
        if(now_check_point->change_num == 0 || now_check_point->now_thresholds < 10) {
            qDebug()<<"(x,y): ("<<now_check_point->col<<","<<now_check_point->row<<")点位检测结束";
            now_check_point->check_over = true;
            return;
        }

        //阈值转折
        if(now_check_point->direact == DOWN){
            //转折需求数量-1

            now_check_point->change_num--;
            now_check_point->direact = UP;
            now_check_point->change_value = -now_check_point->change_value+1;
            qDebug()<<"(x,y): ("<<now_check_point->col<<","<<now_check_point->row<<")该点转折向上,变化量:"<<now_check_point->change_value;
        }

        qDebug()<<"(x,y): ("<<now_check_point->col<<","<<now_check_point->row<<")点位增加亮度";
        now_check_point->early_thresholds = now_check_point->now_thresholds;
        now_check_point->now_thresholds = now_check_point->now_thresholds+now_check_point->change_value;
        now_check_point->check_pair =false;
    }









}

void data_process::False_negative_tests(RectROI *now_check_point)
{

}

void data_process::False_positive_tests(RectROI *now_check_point)
{

}

data_process::data_process()
{

}
