QT += quick

CONFIG += c++14

include(../../qpluginfactory.pri)

HEADERS += \
	pluginhelper.h \
	iplugin.h

SOURCES += main.cpp \
	pluginhelper.cpp

RESOURCES += qml.qrc

ANDROID_EXTRA_PLUGINS = $$OUT_PWD/../plugins
