#ifndef PLUGINHELPER_H
#define PLUGINHELPER_H

#include <QObject>
#include <qpluginfactory.h>
#include "iplugin.h"

class PluginHelper : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QStringList keys READ keys CONSTANT)

	Q_PROPERTY(QString title READ title NOTIFY updated)

public:
	explicit PluginHelper(QObject *parent = nullptr);

	QStringList keys() const;
	QString title() const;

public slots:
	void info(const QString &name);
	void load(const QString &name);

	void ping(const QString &text);

signals:
	void updated();

	void pong(const QString &text);

private:
	IPlugin *_current;
};

#endif // PLUGINHELPER_H
