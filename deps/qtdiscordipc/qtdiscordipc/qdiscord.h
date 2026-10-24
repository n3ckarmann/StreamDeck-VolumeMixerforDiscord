#pragma once

#include <QObject>
#include <QLocalSocket>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QImage>
#include <QCache>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrlQuery>

#include "qdiscordmessage.h"
#include "qdiscordreply.h"

class QDiscord : public QObject {
Q_OBJECT

public:
	static constexpr float minVoiceVolume = 0;
	static constexpr float maxVoiceVolume = 200;

public:
	enum class CommandType {
		unknown = -1,
		dispatch,
		authorize,
		authenticate,

		getGuild,
		getGuilds,
		getChannel,
		getChannels,

		subscribe,
		unsubscribe,

		setUserVoiceSettings,
		selectVoiceChannel,
		getSelectedVoiceChannel,
		selectTextChannel,
		setVoiceSettings,
		getVoiceSettings,
		setCertifiedDevices,

		setActivity,
		sendActivityJoinInvite,
		closeActivityRequest,
	};

	Q_ENUM(CommandType);

public:
	QDiscord();
	~QDiscord();

public:
	/// Maps volume value returned by the IPC to the value that is shown in the Discord UI
	static double ipcToUIVolume(double v);

	/// Maps volume value that is shown in the Discord UI to the value range the IPC uses
	static double uiToIPCVolume(double v);

public:
	/**
	 * Tries to connext to the Discord. Returns true if successfull.
	 * The function does not return until finished, but it processes events while waiting.
	 * Calls made while a connection attempt is already running return false immediately.
	 */
	bool connect(const QString &clientID, const QString &clientSecret);

	void disconnect();

	inline bool isConnected() const {
		return isConnected_;
	}

	inline bool isConnecting() const {
		return connecting_;
	}

	inline const QString &connectionError() const {
		return connectionError_;
	}

	inline const QString &userID() const {
		return userID_;
	}

	/// Returns whether the discord is processing something (connecting, waiting for message, ...)
	inline bool isProcessing() const {
		return processing_ > 0;
	}

public:
	/**
	 * Sends a command. Asynchronously returns the result via QDiscordReply::finished
	 * $args is put in the "args" field.
	 * $msgOverrides is injected into the main body
	 */
	QDiscordReply *sendCommand(const QString &command, const QJsonObject &args = {}, const QJsonObject &msgOverrides = {});

public:
	/// The function can be async, the avatar loading can be delayed and then signalled using avatarReady
	QImage getUserAvatar(const QString &userId, const QString &avatarId);

signals:
	/// This signal is emitted when there is a message received that is not a response to a command
	void messageReceived(const QDiscordMessage &msg);

	void avatarReady(const QString &avatarId, const QImage &img);

	void connected();

	/// Emitted when connectionError() changes during/after a connection attempt
	void connectionStateChanged();

	void disconnected();

private:
	/// Blockingly reads a single message.
	QDiscordMessage readMessage(int timeoutMs = handshakeTimeout);

	/// Posts an OAuth2 token request, returns the parsed JSON response in result
	QNetworkReply::NetworkError requestToken(const QUrlQuery &query, QJsonObject &result);

	void sendMessage(const QJsonObject &packet, int opCode = 1);

	/**
 * Processes incoming messages.
 */
	void processMessage(const QDiscordMessage &msg);

private:
	/// Blocking waits until given amount of bytes is available
	QByteArray blockingReadBytes(int bytes, int timeoutMs);

	/// Non-blocking processes received messsages
	void readAndProcessMessages();

private:
	static constexpr int handshakeTimeout = 5000;
	/// Time the user has to confirm the authorization dialog in Discord
	static constexpr int authorizeTimeout = 120000;
	static constexpr int httpTimeout = 15000;
	static constexpr const char *tokenUrl = "https://discord.com/api/oauth2/token";
	/// Has to match a redirect configured in the Discord app's OAuth2 settings
	static constexpr const char *redirectUri = "http://localhost:1337/callback";

	static QString oauthFilePath();

private:
	QLocalSocket socket_;
	bool isConnected_ = false;
	bool connecting_ = false;
	QString connectionError_;
	QString userID_;
	QString cdn_;
	int nonceCounter_ = 0;
	int blockingRead_ = 0;
	int processing_ = 0;

private:
	QNetworkAccessManager netMgr_;
	QCache<QString, QImage> avatarsCache_;
	QHash<QString, QDiscordReply *> pendingReplies_;

};

QString operator +(QDiscord::CommandType ct);