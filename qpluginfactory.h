#ifndef QPLUGINFACTORY_H
#define QPLUGINFACTORY_H

#include <QDir>
#include <QObject>
#include <QPluginLoader>
#include <QException>

class QPluginLoadException : public QException
{
public:
	QPluginLoadException(QSharedPointer<QPluginLoader> loader);

	const char *what() const noexcept override;
	void raise() const override;
	QException *clone() const override;

private:
	QPluginLoadException(const QByteArray &error);
	const QByteArray _what;
};

class QPluginFactory : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString pluginType READ pluginType CONSTANT)
	Q_PROPERTY(QByteArray pluginIid READ pluginIid WRITE setPluginIid)

public:
	QPluginFactory(const QString &pluginType, QObject *parent = nullptr);
	QPluginFactory(const QString &pluginType, const QByteArray &pluginIid, QObject *parent = nullptr);

	void addSearchDir(const QDir &dir, bool isTopLevel = false);

	QStringList allKeys() const;
	QJsonObject metaData(const QString &key) const;
	QObject *plugin(const QString &key) const;

	QString pluginType() const;
	QByteArray pluginIid() const;

public slots:
	virtual void setPluginIid(const QByteArray &pluginIid);

	void reloadPlugins();

private:
	const QString _pluginType;
	QByteArray _pluginIid;
	QList<QDir> _extraDirs;

	QHash<QString, QSharedPointer<QPluginLoader>> _loaders;
};

template <typename TPlugin>
class QGenericPluginFactory : public QPluginFactory
{
public:
	QGenericPluginFactory(const QString &pluginType, QObject *parent = nullptr);

	TPlugin *plugin(const QString &key) const;
	QObject *pluginObj(const QString &key) const;

	void setPluginIid(const QByteArray &) override;
};

template <typename TPlugin, typename TObject>
class QPluginObjectFactory : public QGenericPluginFactory<TPlugin>
{
public:
	QPluginObjectFactory(const QString &pluginType, QObject *parent = nullptr);

	template <typename... Args>
	TObject *createInstance(const QString &key, Args... args) const;
};

//single-line loader method


// ------------- Template Implementations -------------

template<typename TPlugin>
QGenericPluginFactory<TPlugin>::QGenericPluginFactory(const QString &pluginType, QObject *parent) :
	QPluginFactory(pluginType, qobject_interface_iid<TPlugin>(), parent)
{}

template<typename TPlugin>
TPlugin *QGenericPluginFactory<TPlugin>::plugin(const QString &key) const
{
	return qobject_cast<TPlugin*>(QPluginFactory::plugin(key));
}

template<typename TPlugin>
QObject *QGenericPluginFactory<TPlugin>::pluginObj(const QString &key) const
{
	return QPluginFactory::plugin(key);
}

template<typename TPlugin>
void QGenericPluginFactory<TPlugin>::setPluginIid(const QByteArray &) {}

template<typename TPlugin, typename TObject>
QPluginObjectFactory<TPlugin, TObject>::QPluginObjectFactory(const QString &pluginType, QObject *parent) :
	QGenericPluginFactory<TPlugin>(pluginType, parent)
{}

template<typename TPlugin, typename TObject>
template <typename... Args>
TObject *QPluginObjectFactory<TPlugin, TObject>::createInstance(const QString &key, Args... args) const
{
	auto plg = this->plugin(key);
	if(plg)
		return plg->createInstance(key, args...);
	else
		return nullptr;
}

#endif // QPLUGINFACTORY_H
