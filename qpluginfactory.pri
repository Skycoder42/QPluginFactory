INCLUDEPATH += $$PWD

HEADERS += \
	$$PWD/qpluginfactory.h

SOURCES += \
	$$PWD/qpluginfactory.cpp

!qpmx_static:include($$PWD/qpluginfactory.prc)

TRANSLATIONS +=
