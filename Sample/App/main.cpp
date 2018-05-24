#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "pluginhelper.h"

int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	qputenv("QT_PLUGIN_PATH", (QCoreApplication::applicationDirPath() + "../plugins").toUtf8());

	QGuiApplication app(argc, argv);

	QQmlApplicationEngine engine;
	engine.rootContext()->setContextProperty("plugin", new PluginHelper(qApp));
	engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
	if (engine.rootObjects().isEmpty())
		return -1;

	return app.exec();
}
