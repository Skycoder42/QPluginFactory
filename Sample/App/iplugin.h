#ifndef IPLUGIN_H
#define IPLUGIN_H

#include <QtGlobal>
#include <QObject>

class IPlugin
{
public:
	virtual inline ~IPlugin() = default;

	virtual QString printText() const = 0;

public Q_SLOTS:
	virtual void ping(const QString &base) = 0;

Q_SIGNALS:
	virtual void pong(const QString &result) = 0;
};

#define IPluginIid "de.skycoder42.qpluginloader.sample.IPlugin"
Q_DECLARE_INTERFACE(IPlugin, IPluginIid)

#endif // IPLUGIN_H
