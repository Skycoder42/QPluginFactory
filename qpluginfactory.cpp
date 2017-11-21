#include "qpluginfactory.h"

#include <QLibraryInfo>
#include <QFileInfo>
#include <QDebug>
#include <QJsonArray>

extern bool __qpluginfactory_is_debug();

QPluginFactoryBase::QPluginFactoryBase(const QString &pluginType, QObject *parent) :
	QPluginFactoryBase(pluginType, QByteArray(), parent)
{}

QPluginFactoryBase::QPluginFactoryBase(const QString &pluginType, const QByteArray &pluginIid, QObject *parent) :
	QObject(parent),
	_pluginType(pluginType),
	_pluginIid(pluginIid),
	_extraDirs(),
	_loaders()
{
	reloadPlugins();
}

void QPluginFactoryBase::addSearchDir(const QDir &dir, bool isTopLevel)
{
	if(isTopLevel) {
		auto mDir = dir;
		if(mDir.cd(_pluginType))
			_extraDirs.append(mDir);
	}  else
		_extraDirs.append(dir);
}

QStringList QPluginFactoryBase::allKeys() const
{
	return _loaders.keys();
}

QJsonObject QPluginFactoryBase::metaData(const QString &key) const
{
	auto loader = _loaders.value(key);
	if(loader)
		return loader->metaData()[QStringLiteral("MetaData")].toObject();
	else
		return {};
}

QObject *QPluginFactoryBase::plugin(const QString &key) const
{
	auto loader = _loaders.value(key);
	if(loader) {
		if(!loader->isLoaded() && !loader->load())
			throw QPluginLoadException(loader);
		return loader->instance();
	} else
		return nullptr;
}

QString QPluginFactoryBase::pluginType() const
{
	return _pluginType;
}

QByteArray QPluginFactoryBase::pluginIid() const
{
	return _pluginIid;
}

void QPluginFactoryBase::setPluginIid(const QByteArray &pluginIid)
{
	_pluginIid = pluginIid;
	reloadPlugins();
}

void QPluginFactoryBase::reloadPlugins()
{
	//find the plugin dir
	auto oldKeys = _loaders.keys();

	QList<QDir> allDirs;
	//first: dirs in path
	auto path = qgetenv(QStringLiteral("PLUGIN_%1_PATH").arg(_pluginType.toUpper()).toUtf8().constData());
	if(!path.isEmpty()) {
#ifdef Q_OS_WIN
		const static auto seperator = ';';
#else
		const static auto seperator = ':';
#endif
		foreach(auto p, path.split(seperator)) {
			QDir dir(QString::fromUtf8(p));
			if(dir.exists())
				allDirs.append(dir);
		}
	}

	//second: extra dirs
	allDirs.append(_extraDirs);

	//third: original plugin dir
	QDir pluginMainDir = QLibraryInfo::location(QLibraryInfo::PluginsPath);
	if(pluginMainDir.cd(_pluginType))
		allDirs.append(pluginMainDir);

	foreach(auto pluginDir, allDirs) {
		foreach(auto info, pluginDir.entryInfoList(QDir::Files | QDir::Readable | QDir::Executable | QDir::NoDotAndDotDot)) {
			QSharedPointer<QPluginLoader> plugin(new QPluginLoader(info.absoluteFilePath()), &QObject::deleteLater);
			auto metaData = plugin->metaData();

			//skip non-matching types
			if(metaData[QStringLiteral("debug")].toBool() != __qpluginfactory_is_debug())
				continue;

			auto iid = plugin->metaData().value(QStringLiteral("IID")).toString();
			if(!_pluginIid.isNull() && iid != _pluginIid) {
				qWarning().noquote() << "File" << plugin->fileName() << "is no plugin of type" << _pluginIid;
				continue;
			}

			auto data = metaData[QStringLiteral("MetaData")].toObject();
			auto keys = data[QStringLiteral("Keys")].toArray();
			if(keys.isEmpty()) {
				qWarning().noquote() << "Plugin" << plugin->fileName() << "is does not provide any Keys";
				continue;
			}

			foreach(auto key, keys) {
				auto k = key.toString();
				if(!_loaders.contains(k))
					_loaders.insert(k, plugin);
				oldKeys.removeOne(k);
			}

		}
	}

	foreach(auto key, oldKeys)
		_loaders.remove(key);
}



QPluginLoadException::QPluginLoadException(QSharedPointer<QPluginLoader> loader) :
	QPluginLoadException(QStringLiteral("Failed to load plugin \"%1\" with error: %2")
						 .arg(loader->fileName())
						 .arg(loader->errorString())
						 .toUtf8())
{}

const char *QPluginLoadException::what() const noexcept
{
	return _what.constData();
}

void QPluginLoadException::raise() const
{
	throw *this;
}

QException *QPluginLoadException::clone() const
{
	return new QPluginLoadException(_what);
}

QPluginLoadException::QPluginLoadException(const QByteArray &error) :
	QException(),
	_what(error)
{}
