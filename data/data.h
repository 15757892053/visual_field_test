#ifndef DATA_H
#define DATA_H

#include <QtSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlTableModel>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QVector>

typedef enum{LEFT_EYE,RIGHT_EYE}eye_type;
typedef enum{MAN,FEMALE}gender;
typedef enum{UP,DOWN}change_director;
typedef enum{FAST,SITA}strategy;

typedef struct RectROI {
    int row;//行
    int col;//列

    //保留前一个时刻的测试阈值，用于判断假阴
    int now_thresholds;
    int early_thresholds = 50;
    int test_show_thresholds;

    //断定看不见所需的次数
    int need_count = 1;
    //
    bool nonesee =true;
    //阈值变化方向,阶梯法判断是否到达阈值
    change_director direact = DOWN;
    //双阶梯法记录方向变化的次数
    int change_num;
    //每次变化的值
    int change_value = 3;

    //是否完成数据更新
    bool has_change = false;

    //是否已有按键响应
    bool check_pair = false;

    //检测完成标志与生理盲点标志
    bool check_over = false;
    bool blind_spot = false;
    bool invaild_area = false;

    //处于假阴/假阳测试
    bool in_negative = false;
    bool in_positive = false;
    bool in_check = true;


}RectROI;

typedef struct Test_counting{
    int all_number;
    int positive_test_number;
    int negative_test_number;
    int false_positive_number;
    int false_negative_number;
}Test_Counting;


typedef struct People_message{
    int age;
    int people_ID ;
    gender gender;
    eye_type eye;

}People_info;



class Data_base{
public:
    Data_base();
    void init_database();
    void load_visualdata();
    QSqlDatabase get_database(){return Visual_DB;};
    QVector<QVector<double>> Get_mean_threshold();
    //信息注册
    void set_age(const int age){ people_info.age = age;};
    void set_gender(const gender Gender){people_info.gender = Gender;};
    void set_eyetype(const eye_type eye){people_info.eye = eye;};

    //
    eye_type get_eyetype(){return people_info.eye;};



private:
    QSqlDatabase Visual_DB;
    People_info  people_info;


    //辅助函数
    void caculate_mean_table();
    void creat_datatable(QSqlQuery &query);
    bool deleteTable(const QString& tableName);
    bool calculatemeanAndInsert(eye_type eye);
    void parseAndInsertData(const QByteArray &jsonData);
    QVector<QVector<double>> jsonToMatrix(const QString& jsonStr);
    QString matrixToJson(const QVector<QVector<double>>& matrix);
    QVector<QVector<double>> mergeMatrices(const QVector<QVector<double>>& hvf, const QVector<QVector<double>>& td);
    QVector<QVector<double>> mergeAndAverage(const QVector<QVector<double>> &matrix1, const QVector<QVector<double>> &matrix2, int count);

};





#endif
