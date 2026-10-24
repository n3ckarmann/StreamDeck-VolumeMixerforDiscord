#pragma once

#include <qtstreamdeck2/qstreamdeckplugin.h>
#include <qtdiscordipc/qdiscord.h>
#include <QSettings>

#include "declares.h"
#include "voicechannelmember.h"
#include "dvmdevice.h"

class DVMPlugin : public QStreamDeckPluginT<DVMDevice> {
Q_OBJECT

public:
	DVMPlugin();
	~DVMPlugin();

public slots:
	/// Attempts to connect to Discord
	void connectToDiscord();

	/// Reloads all channel member data
	void updateChannelMembersData();

public:
	/// Localized, human readable text for the current Discord connection problem (for button titles)
	QString connectionStatusText() const;

public:
	static constexpr const char *originalProjectUrl = "https://github.com/CZDanol/StreamDeck-DiscordVolumeMixer2";
	static constexpr const char *setupGuideUrl = "https://github.com/CZDanol/StreamDeck-DiscordVolumeMixer2#configuration";

public:
	/// Processes voice state update for the user itself
	void updateSelfVoiceState(const QDiscordMessage &msg);

	void adjustVoiceChannelMemberVolume(VoiceChannelMember &vcm, float stepSize, int numSteps);

public:
	QDiscord discord;

public:
	QString currentVoiceChannelID;
	QMap<QString, VoiceChannelMember> voiceChannelMembers;
	QSet<QString> speakingVoiceChannelMembers;

	int voiceChannelMemberIxOffset = 0;

public:
	bool isDeafened = false;
	bool isMicrophoneMuted = false;

signals:
	/// Updates text & states of all user related buttons
	void buttonsUpdateRequested();

private:
	void updateCurrentVoiceChannel(const QString &newVoiceChannel);

private slots:
	void onInitialized();
	void onDiscordMessageReceived(const QDiscordMessage &msg);
	void onStreamDeckEventReceived(const QStreamDeckEvent &e);

private:
	static constexpr int minReconnectInterval = 2500;
	static constexpr int maxReconnectInterval = 30000;
	/// While Discord is not running, check more often so we connect quickly after it is started
	static constexpr int maxReconnectIntervalNoDiscord = 5000;

private:
	QTimer discordConnectTimeoutTimer_;
	QTimer discordReconnectTimer_;

};
