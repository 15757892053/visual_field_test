QT       += core gui
QT       += sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../creat_losspic/discheckpoint.cpp \
    data/data.cpp \
    main.cpp \
    mainwindow.cpp \
    process/test_process.cpp \
    result_statistics/result_show.cpp \
    result_widget.cpp \
    test_widget.cpp

HEADERS += \
    ../creat_losspic/discheckpoint.h \
    data/data.h \
    mainwindow.h \
    process/test_process.h \
    result_statistics/result_show.h \
    result_widget.h \
    test_widget.h

FORMS += \
    mainwindow.ui \
    result_widget.ui \
    test_widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
