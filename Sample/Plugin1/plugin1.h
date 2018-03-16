#ifndef PLUGIN1_H
#define PLUGIN1_H

#include "../App/iplugin.h"

class Plugin1 : public QObject, public IPlugin
{
	Q_OBJECT
	Q_INTERFACES(IPlugin)
	Q_PLUGIN_METADATA(IID IPluginIid FILE "Plugin1.json")

public:
	explicit Plugin1(QObject *parent = nullptr);

	QString printText() const override;

public slots:
	void ping(const QString &base) override;

signals:
	void pong(const QString &result) final;
};

#endif // PLUGIN1_H
