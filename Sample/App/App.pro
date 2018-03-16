QT += quick

CONFIG += c++14

include(../../qpluginfactory.pri)

SOURCES += main.cpp \
	pluginhelper.cpp

RESOURCES += qml.qrc

HEADERS += \
	pluginhelper.h \
	iplugin.h

contains(ANDROID_TARGET_ARCH,x86) {
	ANDROID_EXTRA_LIBS = \
		$$OUT_PWD/../Plugin1/libplugins_plugin_Plugin1.so
}
