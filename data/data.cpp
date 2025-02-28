#include <data/data.h>

Data_base::Data_base()
{

}

void Data_base::init_database()
{
    Visual_DB = QSqlDatabase::addDatabase("QSQLITE");
    Visual_DB.setDatabaseName("compary.db");
    //打开数据库
    if(!Visual_DB.open()){
        qDebug() << "open error" <<Visual_DB.lastError();
    }
    else qDebug() << "open success";

    QSqlQuery query;
    // 创建人物表 (person)
    QString sqlCreatePersonTable = QString(
        "CREATE TABLE IF NOT EXISTS person ("
        "person_id INTEGER PRIMARY KEY, "
        "gender TEXT, "
        "year INTEGER "
        ");"
        );
    if (!query.exec(sqlCreatePersonTable)) {
        qDebug() << "create person table error:" << query.lastError();
    } else {
        qDebug() << "create person table success";
    }

    // 创建眼睛表 (eye)
    QString sqlCreateEyeTable = QString(
        "CREATE TABLE IF NOT EXISTS eye ("
        "eye_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "person_id INTEGER, "
        "eye_type TEXT CHECK(eye_type IN ('L', 'R')), "
        "FOREIGN KEY (person_id) REFERENCES person(person_id)"
        ");"
        );
    if (!query.exec(sqlCreateEyeTable)) {
        qDebug() << "create eye table error:" << query.lastError();
    } else {
        qDebug() << "create eye table success";
    }

    // 创建测试数据表 (test_data)
    QString sqlCreateTestDataTable = QString(
        "CREATE TABLE IF NOT EXISTS test_data ("
        "test_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "eye_id REAL, "
        "test_age INTEGER, "
        "hvf TEXT, "  // 存储矩阵数据（JSON 或文本格式）
        "td TEXT, "
        "FOREIGN KEY (eye_id) REFERENCES eye(eye_id)"
        ");"
        );
    if (!query.exec(sqlCreateTestDataTable)) {
        qDebug() << "create test_data table error:" << query.lastError();
    } else {
        qDebug() << "create test_data table success";
    }

    deleteTable("mean_data_R");
    deleteTable("mean_data_L");

    // 创建新表
    QString sqlCreateMean_RTable = QString(
        "CREATE TABLE IF NOT EXISTS mean_data_R ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "test_age REAL, "
        "merged_matrix TEXT "
        ");"
        );
    if (!query.exec(sqlCreateMean_RTable)) {
        qDebug() << "Create new table error:" << query.lastError();
    }else {
        qDebug() << "create mean table success";
    }


    QString sqlCreateMean_LTable = QString(
        "CREATE TABLE IF NOT EXISTS mean_data_L ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "test_age REAL, "
        "merged_matrix TEXT "
        ");"
        );

    if (!query.exec(sqlCreateMean_LTable)) {
        qDebug() << "Create new table error:" << query.lastError();
    }else {
        qDebug() << "create mean table success";
    }




#if 1
    QString sqlSelect = "SELECT test_age, hvf, td ,eye_id  FROM test_data ORDER BY test_age ASC";
    if (!query.exec(sqlSelect)) {
        qDebug() << "Select data error:" << query.lastError();
    }

    QSqlQuery insertQuery;
    // 处理数据并插入到新表中
    while (query.next()) {
        int eye_id = query.value("eye_id").toInt();
        double testAge = query.value("test_age").toDouble();
        QString hvfStr = query.value("hvf").toString();
        QString tdStr = query.value("td").toString();

        QString Get_eyetype = "SELECT eye_type FROM eye WHERE eye_id = :eye_id";
        insertQuery.prepare(Get_eyetype);
        insertQuery.bindValue(":eye_id", eye_id);
        if (!insertQuery.exec()) {
            qDebug() << "Select eye type error:" << insertQuery.lastError();
            continue;
        }
        QString eye_type;
        if(insertQuery.next()){
            eye_type = insertQuery.value("eye_type").toString();
        }
        else qDebug() << "Read eye type error" ;
        QString insertTableName;
        if (eye_type == "L") {
            insertTableName = "mean_data_L";
        } else if (eye_type == "R") {
            insertTableName = "mean_data_R";
        } else {
            qDebug() << "Invalid eye type: " << eye_type;
            continue;
        }
        insertQuery.clear();



        // 将 JSON 字符串转换为二维矩阵
        QVector<QVector<double>> hvfMatrix = jsonToMatrix(hvfStr);
        QVector<QVector<double>> tdMatrix = jsonToMatrix(tdStr);

        // 合并矩阵
        QVector<QVector<double>> mergedMatrix = mergeMatrices(hvfMatrix, tdMatrix);

        // 将合并后的矩阵转换为 JSON 字符串
        QString mergedMatrixStr = matrixToJson(mergedMatrix);

        // 插入到新表中
        QString sqlInsert = QString("INSERT INTO %1 (test_age, merged_matrix) VALUES (:test_age, :merged_matrix)").arg(insertTableName);
        insertQuery.prepare(sqlInsert);
        insertQuery.bindValue(":test_age", testAge);
        insertQuery.bindValue(":merged_matrix", mergedMatrixStr);

//      qDebug() << "Binding values: test_age =" << testAge << ", merged_matrix =" << mergedMatrixStr;
        if (!insertQuery.exec()) {
            qDebug() << "Insert data error:" << query.lastError();
        }


    }

    qDebug() << "Data processing and insertion completed.";
#endif


//    //读取解析json文件

//    QFile file("E:\\visual loss remapping\\visual_field_test\\alldata.json");
//    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
//        qDebug() << "Failed to open JSON file:" << file.errorString();
//        return;
//    }

//    QByteArray jsonData = file.readAll();
//    file.close();

//    parseAndInsertData(jsonData);



}

void Data_base::parseAndInsertData(const QByteArray &jsonData)
{
    // 解析 JSON 数据
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (doc.isNull()) {
        qDebug() << "Failed to parse JSON data.";
        return;
    }

    QJsonObject root = doc.object();

    // 获取 "data" 对象
    QJsonObject data = root["data"].toObject();

    // 遍历每个患者
    for (auto it = data.begin(); it != data.end(); ++it) {
        QString patientId = it.key();  // 患者 ID
        QJsonObject patientData = it.value().toObject();

        // 插入患者信息到 person 表
        QString gender = patientData["gender"].toString();
        int year = patientData["year"].toInt();

        QSqlQuery query;
        query.prepare("INSERT INTO person (person_id, gender, year) VALUES (:person_id, :gender, :year)");
        query.bindValue(":person_id", patientId.toInt());
        query.bindValue(":gender", gender);
        query.bindValue(":year", year);

        if (!query.exec()) {
            qDebug() << "Failed to insert into person table:" << query.lastError();
            continue;
        }

        // 处理左眼和右眼数据
        QStringList eyes = {"L", "R"};
        for (const QString &eyeType : eyes) {
            if (patientData.contains(eyeType)) {
                QJsonArray eyeTests = patientData[eyeType].toArray();

                // 插入眼睛信息到 eye 表
                query.prepare("INSERT INTO eye (person_id, eye_type) VALUES (:person_id, :eye_type)");
                query.bindValue(":person_id", patientId.toInt());
                query.bindValue(":eye_type", eyeType);

                if (!query.exec()) {
                    qDebug() << "Failed to insert into eye table:" << query.lastError();
                    continue;
                }

                int eyeId = query.lastInsertId().toInt();  // 获取插入的 eye_id

                // 插入测试数据到 test_data 表
                for (const QJsonValue &testValue : eyeTests) {
                    QJsonObject test = testValue.toObject();
                    float age = test["age"].toDouble();
                    QJsonArray hvf = test["hvf"].toArray();
                    QJsonArray td = test["td"].toArray();

                    // 将 hvf 和 td 转换为字符串存储
                    QString hvfStr = QJsonDocument(hvf).toJson(QJsonDocument::Compact);
                    QString tdStr = QJsonDocument(td).toJson(QJsonDocument::Compact);

                    query.prepare("INSERT INTO test_data (eye_id, test_age, hvf, td) "
                                  "VALUES (:eye_id, :test_age, :hvf, :td)");
                    query.bindValue(":eye_id", eyeId);
                    query.bindValue(":test_age", age);
                    query.bindValue(":hvf", hvfStr);
                    query.bindValue(":td", tdStr);

                    if (!query.exec()) {
                        qDebug() << "Failed to insert into test_data table:" << query.lastError();
                    }
                }
            }
        }
    }
}

bool Data_base::deleteTable(const QString &tableName)
{
    // 创建 QSqlQuery 对象
    QSqlQuery query;

    // 执行删除表格的 SQL 语句
    QString sqlDropTable = QString("DROP TABLE IF EXISTS %1").arg(tableName);
    if (!query.exec(sqlDropTable)) {
        qDebug() << "Drop table error:" << query.lastError();
        Visual_DB.close();
        return false;
    }
    qDebug() << "Table " << tableName << " dropped successfully.";
    return true;
}

QVector<QVector<double> > Data_base::jsonToMatrix(const QString &jsonStr)
{
    QVector<QVector<double>> matrix;
    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
    QJsonArray outerArray = doc.array();
    for (const auto& innerValue : outerArray) {
        QJsonArray innerArray = innerValue.toArray();
        QVector<double> row;
        for (const auto& value : innerArray) {
            row.append(value.toDouble());
        }
        matrix.append(row);
    }
    return matrix;
}

QString Data_base::matrixToJson(const QVector<QVector<double> > &matrix)
{
    QJsonArray outerArray;
    for (const auto& row : matrix) {
        QJsonArray innerArray;
        for (double value : row) {
            // 保留两位小数并四舍五入
            QString formattedValue = QString::number(value, 'f', 2);
            innerArray.append(formattedValue.toDouble());
        }
        outerArray.append(innerArray);
    }
    QJsonDocument doc(outerArray);
    return doc.toJson(QJsonDocument::Compact);
}

QVector<QVector<double> > Data_base::mergeMatrices(const QVector<QVector<double> > &hvf, const QVector<QVector<double> > &td)
{
    QVector<QVector<double>> result;
    for (int i = 0; i < hvf.size(); ++i) {
        QVector<double> row;
        for (int j = 0; j < hvf[i].size(); ++j) {
            row.append(hvf[i][j] - td[i][j]);
        }
        result.append(row);
    }
    return result;
}


