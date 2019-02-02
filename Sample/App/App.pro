QT += quick

CONFIG += c++14

HEADERS += \
	pluginhelper.h \
	iplugin.h

SOURCES += main.cpp \
	pluginhelper.cpp

RESOURCES += qml.qrc

ANDROID_EXTRA_PLUGINS = $$OUT_PWD/../plugins

include(../../qpluginfactory.pri)
!load(qdep):error("qdep required")
