#include "dvmplugin.h"

#include <QDir>
#include <QFile>
#include <QSettings>
#include <QGuiApplication>
#include <QThread>
#include <QSslSocket>

void messageLogger(QtMsgType t, const QMessageLogContext &, const QString &msg) {
	static QFile lf("log.txt"), clf("clog.txt");

	// Only create log files if they already exist
	if(!lf.isOpen() && lf.exists())
		lf.open(QIODevice::WriteOnly);

	if(!clf.isOpen() && clf.exists())
		clf.open(QIODevice::Append);

	if(lf.isOpen()) {
		lf.write(msg.toUtf8());
		lf.write("\n");
		lf.flush();
	}

	if(clf.isOpen()) {
		clf.write(msg.toUtf8());
		clf.write("\n");
		clf.flush();
	}
}

/// The plugin used to be published as cz.danol.discordmixer. Take over its Client ID/secret and Discord login,
/// so switching to this plugin doesn't need setting up again (it has to be installed while the old one still is).
void migrateFromOldPlugin(const QString &pluginUID) {
	static const QString oldPluginUID = QStringLiteral("cz.danol.discordmixer");

	QSettings settings(QSettings::UserScope, "Elgato Stream Deck Plugin", pluginUID);
	QSettings oldSettings(QSettings::UserScope, "Elgato Stream Deck Plugin", oldPluginUID);
	if(!settings.contains("globalSettings") && oldSettings.contains("globalSettings")) {
		qDebug() << "Taking over settings of" << oldPluginUID;
		settings.setValue("globalSettings", oldSettings.value("globalSettings"));
	}

	// <plugins dir>/<plugin>.sdPlugin/bin/<exe>
	const QDir pluginsDir(QCoreApplication::applicationDirPath() + "/../..");
	const QString oauthFile = pluginsDir.absoluteFilePath(pluginUID + ".sdPlugin/discordOauth.json");
	const QString oldOauthFile = pluginsDir.absoluteFilePath(oldPluginUID + ".sdPlugin/discordOauth.json");
	if(!QFile::exists(oauthFile) && QFile::exists(oldOauthFile) && QFile::copy(oldOauthFile, oauthFile))
		qDebug() << "Took over Discord login of" << oldPluginUID;
}

int main(int argc, char *argv[]) {
	QCoreApplication::setAttribute(Qt::AA_PluginApplication);

	//QThread::sleep(10);

	QGuiApplication app(argc, argv);
	qInstallMessageHandler(&messageLogger);
	qDebug() << QDateTime::currentDateTime().toString() << "Plugin starting";

	// Use the TLS implementation built into Windows, so no OpenSSL DLLs have to be shipped (and kept up to date)
	if(QSslSocket::availableBackends().contains("schannel"))
		QSslSocket::setActiveBackend("schannel");
	qDebug() << "TLS backend" << QSslSocket::activeBackend();

	const QString pluginUID = QStringLiteral("com.n3ckarmann.volumemixer");
	migrateFromOldPlugin(pluginUID);

	DVMPlugin plugin;
	plugin.init(pluginUID, app);

	return QGuiApplication::exec();
}