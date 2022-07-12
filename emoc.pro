QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 console

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/main.cpp \
    src/emoc.cpp \
    src/slider.cpp \
    src/touch_button.cpp \
    src/viewer.cpp

HEADERS += \
    src/emoc.hpp \
    src/slider.hpp \
    src/touch_button.hpp \
    src/viewer.hpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

unix: LIBS += -L/usr/lib/ -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_imgcodecs

unix:INCLUDEPATH += /usr/include/opencv4
unix:DEPENDPATH += /usr/local/include/opencv4
