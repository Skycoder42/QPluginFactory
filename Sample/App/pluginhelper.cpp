#include "pluginhelper.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDirIterator>
#include <QStandardPaths>
#include <QLibraryInfo>

Q_GLOBAL_PLUGIN_FACTORY(IPlugin, "plugin", loader)

PluginHelper::PluginHelper(QObject *parent) :
	QObject(parent),
	_current(nullptr)
{}

QStringList PluginHelper::keys() const
{
	return loader->allKeys();
}

QString PluginHelper::title() const
{
	return _current ? _current->printText() : QString();
}

void PluginHelper::info(const QString &name)
{
	qInfo() << loader->metaData(name);
}

void PluginHelper::load(const QString &name)
{
	if(_current)
		dynamic_cast<QObject*>(_current)->disconnect(this);
	_current = loader->plugin(name);
	connect(dynamic_cast<QObject*>(_current), SIGNAL(pong(QString)),
			this, SIGNAL(pong(QString)));
	emit updated();
}

void PluginHelper::ping(const QString &text)
{
	if(_current)
		_current->ping(text);
}
