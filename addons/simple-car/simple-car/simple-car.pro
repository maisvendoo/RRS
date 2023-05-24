# Шаблон проекта - динамическая библиотека (DLL)
TEMPLATE = lib
# Задаем те библиотеки QT, которые нам пригодятся
QT -= gui
QT += xml

# Имя итогового файла DLL и путь, куда он должен быть помещен после сборки
TARGET = simple-car

DESTDIR = $$(RRS_DEV_ROOT)/modules/$$join(TARGET,,,)

# Библиотеки симулятора, с которыми компонуется DLL ПЕ
    LIBS += -L../../../../lib -lCfgReader
    LIBS += -L../../../../lib -lphysics
    LIBS += -L../../../../lib -lvehicle
    LIBS += -L../../../../lib -ldevice
    LIBS += -L../../../../lib -lfilesystem

# Путь к необходимым заголовочным файлам
INCLUDEPATH += ./include
INCLUDEPATH += ../../../CfgReader/include

INCLUDEPATH += ../../../simulator/solver/include
INCLUDEPATH += ../../../simulator/physics/include
INCLUDEPATH += ../../../simulator/vehicle/include
INCLUDEPATH += ../../../simulator/device/include
INCLUDEPATH += ../../../filesystem/include

# Указываем файлы, включаемые в проект
HEADERS += $$files(./include/*.h)
SOURCES += $$files(./src/*.cpp)
