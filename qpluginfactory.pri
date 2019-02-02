INCLUDEPATH += $$PWD

HEADERS += \
	$$PWD/qpluginfactory.h

SOURCES += \
	$$PWD/qpluginfactory.cpp

QDEP_DEPENDS += Skycoder42/QExceptionBase

QDEP_PACKAGE_EXPORTS += Q_PLUGIN_FACTORY_EXPORT
!qdep_build: DEFINES += "Q_PLUGIN_FACTORY_EXPORT="
