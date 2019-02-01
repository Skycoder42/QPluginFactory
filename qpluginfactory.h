#ifndef QPLUGINFACTORY_H
#define QPLUGINFACTORY_H

#include <QtCore/QDir>
#include <QtCore/QObject>
#include <QtCore/QPluginLoader>
#include <QtCore/QException>
#include <QtCore/QMutex>

#ifdef QT_NO_DEBUG
#define Q_PLUGIN_FACTORY_IS_DEBUG false
#else
#define Q_PLUGIN_FACTORY_IS_DEBUG true
#endif

class Q_PLUGIN_FACTORY_EXPORT QPluginLoadException : public QException
{
public:
	QPluginLoadException(QPluginLoader *loader);

	const char *what() const noexcept override;
	void raise() const override;
	QException *clone() const override;

private:
	QPluginLoadException(QByteArray error);
	const QByteArray _what;
};

class Q_PLUGIN_FACTORY_EXPORT QPluginFactoryBase : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString pluginType READ pluginType CONSTANT)
	Q_PROPERTY(QByteArray pluginIid READ pluginIid WRITE setPluginIid)

public:
	class Q_PLUGIN_FACTORY_EXPORT PluginInfo {
		Q_DISABLE_COPY(PluginInfo)
	public:
		inline PluginInfo() = default;
		virtual inline ~PluginInfo() = default;
		virtual QJsonObject metaData() const = 0;
		virtual QObject *instance() = 0;
	};

	QPluginFactoryBase(QString pluginType, QObject *parent = nullptr, bool isDebugBuild = Q_PLUGIN_FACTORY_IS_DEBUG);
	QPluginFactoryBase(QString pluginType, QByteArray pluginIid, QObject *parent = nullptr, bool isDebugBuild = Q_PLUGIN_FACTORY_IS_DEBUG);

	void addSearchDir(const QDir &dir, bool isTopLevel = false);

	QStringList allKeys() const;
	QJsonObject metaData(const QString &key) const;
	QObject *plugin(const QString &key) const;

	QString pluginType() const;
	QByteArray pluginIid() const;

public Q_SLOTS:
	virtual void setPluginIid(const QByteArray &pluginIid);

	void reloadPlugins();

protected:
	bool isLoaded(const QString &key) const;
	void unload(const QString &key);

private:
	const bool _isDebugBuild;
	const QString _pluginType;
	QByteArray _pluginIid;
	QList<QDir> _extraDirs;

	mutable QMutex _loaderMutex;
	QHash<QString, QSharedPointer<PluginInfo>> _plugins;

	QJsonArray checkMeta(const QJsonObject &metaData, const QString &filename) const;
};

template <typename TPlugin>
class QPluginFactory : public QPluginFactoryBase
{
public:
	QPluginFactory(const QString &pluginType, QObject *parent = nullptr);

	TPlugin *plugin(const QString &key) const;
	QObject *pluginObj(const QString &key) const;

	void setPluginIid(const QByteArray &) override;
};

template <typename TPlugin, typename TObject>
class QPluginObjectFactory : public QPluginFactory<TPlugin>
{
public:
	QPluginObjectFactory(const QString &pluginType, QObject *parent = nullptr);

	template <typename... Args>
	TObject *createInstance(const QString &key, Args... args) const;
};

//single-line loader method


// ------------- Template Implementations -------------

template<typename TPlugin>
QPluginFactory<TPlugin>::QPluginFactory(const QString &pluginType, QObject *parent) :
	QPluginFactoryBase(pluginType, qobject_interface_iid<TPlugin*>(), parent)
{}

template<typename TPlugin>
TPlugin *QPluginFactory<TPlugin>::plugin(const QString &key) const
{
	return qobject_cast<TPlugin*>(QPluginFactoryBase::plugin(key));
}

template<typename TPlugin>
QObject *QPluginFactory<TPlugin>::pluginObj(const QString &key) const
{
	return QPluginFactoryBase::plugin(key);
}

template<typename TPlugin>
void QPluginFactory<TPlugin>::setPluginIid(const QByteArray &) {}

template<typename TPlugin, typename TObject>
QPluginObjectFactory<TPlugin, TObject>::QPluginObjectFactory(const QString &pluginType, QObject *parent) :
	QPluginFactory<TPlugin>(pluginType, parent)
{}

template<typename TPlugin, typename TObject>
template<typename... Args>
TObject *QPluginObjectFactory<TPlugin, TObject>::createInstance(const QString &key, Args... args) const
{
	auto plg = this->plugin(key);
	if(plg)
		return plg->createInstance(key, args...);
	else
		return nullptr;
}

#define Q_GLOBAL_PLUGIN_FACTORY(PluginType, pluginKey, instName) namespace { \
	typedef QPluginFactory<PluginType> __QGPF_ ## PluginType ## _Factory; \
	Q_GLOBAL_STATIC_WITH_ARGS(__QGPF_ ## PluginType ## _Factory, instName, (QString::fromUtf8(pluginKey))) \
}

#define Q_GLOBAL_PLUGIN_OBJECT_FACTORY(PluginType, ObjectType, pluginKey, instName) namespace { \
	typedef QPluginObjectFactory<PluginType, ObjectType> __QGPF_ ## PluginType ## _Factory; \
	Q_GLOBAL_STATIC_WITH_ARGS(__QGPF_ ## PluginType ## _Factory, instName, (QString::fromUtf8(pluginKey))) \
}

#endif // QPLUGINFACTORY_H
