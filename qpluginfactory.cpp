#include "qpluginfactory.h"

#include <QFileInfo>
#include <QDebug>
#include <QJsonArray>
#include <QDir>
#include <QCoreApplication>

class StaticPluginInfo : public QPluginFactoryBase::PluginInfo
{
public:
	StaticPluginInfo(const QStaticPlugin &plugin);

	QJsonObject metaData() const override;
	QObject *instance() override;

private:
	QStaticPlugin _plugin;
};

class DynamicPluginInfo : public QPluginFactoryBase::PluginInfo
{
public:
	DynamicPluginInfo(QScopedPointer<QPluginLoader, QScopedPointerDeleteLater> &loader);

	QJsonObject metaData() const override;
	QObject *instance() override;

	QPluginLoader *loader() const;

private:
	QScopedPointer<QPluginLoader, QScopedPointerDeleteLater> _loader;
};



QPluginFactoryBase::QPluginFactoryBase(QString pluginType, QObject *parent, bool isDebugBuild) :
	QPluginFactoryBase(std::move(pluginType), QByteArray(), parent, isDebugBuild)
{}

QPluginFactoryBase::QPluginFactoryBase(QString pluginType, QByteArray pluginIid, QObject *parent, bool isDebugBuild) :
	QObject(parent),
	_isDebugBuild{isDebugBuild},
	_pluginType(std::move(pluginType)),
	_pluginIid(std::move(pluginIid)),
	_extraDirs(),
	_loaderMutex(),
	_plugins()
{
	//setup dynamic plugins
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
	QMutexLocker _(&_loaderMutex);
	return _plugins.keys();
}

QJsonObject QPluginFactoryBase::metaData(const QString &key) const
{
	QMutexLocker _(&_loaderMutex);
	auto info = _plugins.value(key);
	if(info)
		return info->metaData()[QStringLiteral("MetaData")].toObject();
	else
		return {};
}

QObject *QPluginFactoryBase::plugin(const QString &key) const
{
	QMutexLocker _(&_loaderMutex);
	auto info = _plugins.value(key);
	if(info)
		return info->instance();
	else
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
	QMutexLocker _(&_loaderMutex);

	//find the plugin dir
	auto oldKeys = _plugins.keys();

	QList<QDir> allDirs;
	//first: dirs in path
	//MAJOR remove
	auto envVar = QStringLiteral("PLUGIN_%1_PATH").arg(_pluginType.toUpper()).toUtf8();
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
	auto path = qEnvironmentVariable(envVar.constData());
#else
	auto path = QString::fromUtf8(qgetenv(envVar.constData()));
#endif
	for(const auto &p : path.split(QDir::listSeparator(), QString::SkipEmptyParts)) {
		QDir dir(p);
		if(dir.exists())
			allDirs.append(dir);
	}

	//second: extra dirs
	allDirs.append(_extraDirs);

	//third: original plugin dirs
	for(const auto &plgPath : QCoreApplication::libraryPaths()) {
		QDir plgDir {plgPath};
		if(plgDir.cd(_pluginType))
			allDirs.append(plgDir);
	}

	//setup dynamic plugins
	for(const auto &pluginDir : allDirs) {
#ifdef Q_OS_WIN
		for(const auto &info : pluginDir.entryInfoList({QStringLiteral("*.dll")}, QDir::Files | QDir::Readable)) {
#else
		for(const auto &info : pluginDir.entryInfoList(QDir::Files | QDir::Readable)) {
#endif
			QScopedPointer<QPluginLoader, QScopedPointerDeleteLater> loader(new QPluginLoader(info.absoluteFilePath()));
			auto metaData = loader->metaData();
			auto keys = checkMeta(metaData, loader->fileName());
			if(keys.isEmpty())
				continue;

			auto dynInfo = QSharedPointer<DynamicPluginInfo>::create(loader);
			for(const auto key : keys) {
				auto k = key.toString();
				if(!_plugins.contains(k))
					_plugins.insert(k, dynInfo);
				oldKeys.removeOne(k);
			}
		}
	}

	//setup static plugins
	for(const auto &info : QPluginLoader::staticPlugins()) {
		auto keys = checkMeta(info.metaData(), QString());
		for(const auto key : keys) {
			auto k = key.toString();
			if(!_plugins.contains(k))
				_plugins.insert(k, QSharedPointer<StaticPluginInfo>::create(info));
			oldKeys.removeOne(k);
		}
	}

	//remove old, now unused plugins
	for(const auto &key : oldKeys)
		_plugins.remove(key);
}

bool QPluginFactoryBase::isLoaded(const QString &key) const
{
	QMutexLocker _(&_loaderMutex);
	auto info = _plugins.value(key);
	if(info) {
		auto dynInfo = info.dynamicCast<DynamicPluginInfo>();
		if(dynInfo)
			return dynInfo->loader()->isLoaded();
		else //static plugin
			return true;
	} else
		return false;
}

void QPluginFactoryBase::unload(const QString &key)
{
	QMutexLocker _(&_loaderMutex);
	auto info = _plugins.value(key);
	if(info) {
		auto dynInfo = info.dynamicCast<DynamicPluginInfo>();
		if(dynInfo) {
			auto loader = dynInfo->loader();
			if(loader->isLoaded()){
				if(!loader->unload())
					qWarning().noquote() << "Failed to unload plugin for key" << key << "with error:" << loader->errorString();
			}
		}
	}
}

QJsonArray QPluginFactoryBase::checkMeta(const QJsonObject &metaData, const QString &filename) const
{
	if(metaData[QStringLiteral("debug")].toBool() != _isDebugBuild)
		return {};

	auto iid = metaData.value(QStringLiteral("IID")).toString().toUtf8();
	if(!_pluginIid.isNull() && iid != _pluginIid) {
		qWarning().noquote() << "File" << filename << "is no plugin of type" << _pluginIid;
		return {};
	}

	auto data = metaData[QStringLiteral("MetaData")].toObject();
	auto keys = data[QStringLiteral("Keys")].toArray();
	if(keys.isEmpty()) {
		qWarning().noquote() << "Plugin" << filename << "is does not provide any Keys";
		return {};
	}

	return keys;
}



QPluginLoadException::QPluginLoadException(QPluginLoader *loader) :
	QPluginLoadException(QStringLiteral("Failed to load plugin \"%1\" with error: %2")
						 .arg(loader->fileName(), loader->errorString())
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

QPluginLoadException::QPluginLoadException(QByteArray error) :
	QException(),
	_what(std::move(error))
{}



StaticPluginInfo::StaticPluginInfo(const QStaticPlugin &plugin) :
	_plugin(plugin)
{}

QJsonObject StaticPluginInfo::metaData() const
{
	return _plugin.metaData();
}

QObject *StaticPluginInfo::instance()
{
	return _plugin.instance();
}

DynamicPluginInfo::DynamicPluginInfo(QScopedPointer<QPluginLoader, QScopedPointerDeleteLater> &loader) :
	_loader()
{
	_loader.swap(loader);
}

QJsonObject DynamicPluginInfo::metaData() const
{
	return _loader->metaData();
}

QObject *DynamicPluginInfo::instance()
{
	if(!_loader->isLoaded() && !_loader->load())
		throw QPluginLoadException(_loader.data());
	return _loader->instance();
}

QPluginLoader *DynamicPluginInfo::loader() const
{
	return _loader.data();
}
