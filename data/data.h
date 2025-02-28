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

typedef struct RectROI {
    int row;//行
    int col;//列

    //保留前一个时刻的测试阈值，用于判断假阴
    int now_thresholds;
    int early_thresholds;

    //阈值变化方向,阶梯法判断是否到达阈值
    bool direact;
    int change_num;

    //检测完成标志与生理盲点标志
    bool check_over = false;
    bool blind_spot = false;
    bool invaild_area = false;
}RectROI;

typedef enum{LEFT_EYE,RIGHT_EYE}eye_type;


typedef struct People_message{
    int people_ID ;
    eye_type eye;
    int age;
}People_info;



class Data_base{
public:
    Data_base();
    void init_database();
    void load_visualdata();
    QSqlDatabase get_database(){return Visual_DB;};
    void parseAndInsertData(const QByteArray &jsonData);
    void caculate_mean_table();

    bool deleteTable(const QString& tableName);


private:
    QSqlDatabase Visual_DB;
    //辅助函数
    QVector<QVector<double>> jsonToMatrix(const QString& jsonStr);
    QString matrixToJson(const QVector<QVector<double>>& matrix);
    QVector<QVector<double>> mergeMatrices(const QVector<QVector<double>>& hvf, const QVector<QVector<double>>& td);

};





#endif
