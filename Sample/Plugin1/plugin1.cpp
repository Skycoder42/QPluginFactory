#include "plugin1.h"

Plugin1::Plugin1(QObject *parent) :
	QObject(parent),
	IPlugin()
{}

QString Plugin1::printText() const
{
	return QStringLiteral("plugin 1");
}

void Plugin1::ping(const QString &base)
{
	emit pong(base.simplified());
}
