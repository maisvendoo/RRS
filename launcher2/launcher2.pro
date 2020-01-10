QT += core qml quick xml
QT -= gui

DEFINES += QT_DEPRECATED_WARNINGS

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

TARGET = launcher2
TEMPLATE = app

DESTDIR = ../../bin

DEFINES += QT_DEPRECATED_WARNINGS
CONFIG += c++11

win32 {

    CONFIG(debug, debug|release) {

        TARGET = $$join(TARGET,,,_d)
        LIBS += -L../../lib -lfilesystem_d
        LIBS += -L../../lib -lCfgReader_d

    } else {
        LIBS += -L../../lib -lfilesystem
        LIBS += -L../../lib -lCfgReader
    }
}

unix {

    CONFIG(debug, debug|release) {

        TARGET = $$join(TARGET,,,_d)
        LIBS += -L../../lib -lfilesystem_d
        LIBS += -L../../lib -lCfgReader_d

    } else {
        LIBS += -L../../lib -lfilesystem
        LIBS += -L../../lib -lCfgReader
    }
}

INCLUDEPATH += ./include
INCLUDEPATH += ../filesystem/include
INCLUDEPATH += ../CfgReader/include

SOURCES   = \
            $$files(./src/*.cpp) \
            $$files(./src/models/*.cpp)
HEADERS   = \
            $$files(./include/*.h) \
            $$files(./include/models/*.h)

lupdate_only{
SOURCES = \
            $$files(./qml/*.qml) \
            $$files(./qml/BaseElements/*.qml) \
            $$files(./qml/CustomElements/*.qml) \
            qml/ApplicationIsRunning.qml \
            qml/BaseElements/BaseButton.qml
}

TRANSLATIONS += translations/launcher.ru_RU.ts
RESOURCES += $$files(*.qrc)
