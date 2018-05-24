QT       += core

TARGET = Plugin1
DESTDIR = ../plugins/plugin/

TEMPLATE = lib
CONFIG += plugin

HEADERS += \
		plugin1.h

SOURCES += \
		plugin1.cpp

DISTFILES += Plugin1.json
