#include "action_vcminfo.h"

#include <QPainter>

#include <qtstreamdeck2/qstreamdeckpropertyinspectorbuilder.h>

#include "dvmplugin.h"
#include "i18n.h"

Action_VCMInfo::Action_VCMInfo() {
	connect(this, &QStreamDeckAction::keyDown, this, &Action_VCMInfo::onPressed);
	connect(this, &QStreamDeckAction::keyUp, this, &Action_VCMInfo::onReleased);

	connect(this, &QStreamDeckAction::dialPressed, this, &Action_VCMInfo::onPressed);
	connect(this, &QStreamDeckAction::dialRotated, this, &Action_VCMInfo::onRotated);
	connect(this, &QStreamDeckAction::touchTap, this, &Action_VCMInfo::onTapped);
}

void Action_VCMInfo::update() {
	if(controller() == Controller::keypad)
		update_button();
	else
		update_encoder();
}

void Action_VCMInfo::update_button() {
	const auto vcmp = voiceChannelMember();
	const VoiceChannelMember &vcm = vcmp ? *vcmp.mem : VoiceChannelMember::null;

	const bool isSpeaking = vcmp && plugin()->speakingVoiceChannelMembers.contains(vcm.userID);

	const QString volumeStr = vcm.isMuted ? I18n::tr("title.muted") : QStringLiteral("%1 %").arg(QString::number(vcm.volume));

	{
		QString newTitle;
		if(!plugin()->discord.isConnected())
			newTitle = plugin()->connectionStatusText();
		else if(vcmp)
			newTitle = QStringLiteral("%1\n%3\n%2").arg(vcm.nick, volumeStr, isSpeaking ? I18n::tr("title.speaking") : "");
		else if(plugin()->voiceChannelMembers.isEmpty() && !plugin()->globalSetting("hideNobodyInVoiceChatText").toBool())
			newTitle = I18n::tr("title.nobodyKey");

		if(title_ != newTitle) {
			title_ = newTitle;
			setTitle(newTitle);
		}
	}

	const QString newUserId = vcm.userID;
	if(userID_ != newUserId || !hasAvatar_) {
		userID_ = newUserId;

		const QImage avatar = plugin()->discord.getUserAvatar(userID_, vcm.avatarID);
		hasAvatar_ = !avatar.isNull();

		QImage img(72, 72, QImage::Format_ARGB32);
		QPainter p(&img);
		img.fill(Qt::transparent);
		if(hasAvatar_) {
			p.setOpacity(0.5);
			p.drawImage(0, 0, avatar.scaled(72, 72, Qt::KeepAspectRatio, Qt::SmoothTransformation));
			p.setOpacity(1);
		}
		setImage(img, 0);

		static const QImage speakingDeco("icons/speaking_deco.png");
		p.drawImage(0, 0, speakingDeco);
		setImage(img, 1);
	}

	const int newState = isSpeaking ? 1 : 0;
	if(state_ != newState) {
		state_ = newState;
		setState(state_);
	}
}

void Action_VCMInfo::update_encoder() {
	const auto vcmp = voiceChannelMember();
	const VoiceChannelMember &vcm = vcmp ? *vcmp.mem : VoiceChannelMember::null;

	const bool isSpeaking = vcmp && plugin()->speakingVoiceChannelMembers.contains(vcm.userID);

	QJsonObject feedbackData;

	{
		QString newTitle;
		if(!plugin()->discord.isConnected())
			newTitle = QString(plugin()->connectionStatusText()).replace('\n', ' ');
		else if(vcmp) {
			if(setting("showPaging").toBool())
				newTitle += QStringLiteral("%1/%2 ").arg(QString::number(vcmp.userIndex + 1), QString::number(plugin()->voiceChannelMembers.size()));

			newTitle += vcm.nick;
		}
		else if(plugin()->voiceChannelMembers.isEmpty() && !plugin()->globalSetting("hideNobodyInVoiceChatText").toBool())
			newTitle = I18n::tr("title.nobodyDial");

		if(newTitle != title_) {
			title_ = newTitle;
			feedbackData.insert("title", newTitle);
		}
	}

	{
		const QString newLayout = vcmp ? "$B1" : "$X1";
		if(feedbackLayout_ != newLayout) {
			feedbackLayout_ = newLayout;
			setFeedbackLayout(newLayout);
		}
	}

	const QString newUserId = vcm.userID;
	if(userID_ != newUserId || !hasAvatar_) {
		userID_ = newUserId;

		const QImage avatar = plugin()->discord.getUserAvatar(userID_, vcm.avatarID);
		hasAvatar_ = !avatar.isNull();

		QImage img(48, 48, QImage::Format_ARGB32);
		QPainter p(&img);
		img.fill(Qt::transparent);
		if(hasAvatar_) {
			p.drawImage(0, 0, avatar.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
		}
		else if(vcmp) {
			static const QImage avatarPlaceholder = QImage("icons/user.png").scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation);
			p.drawImage(0, 0, avatarPlaceholder);
		}

		feedbackData.insert("icon", QStreamDeckPlugin::encodeImage(img));
	}

	const int newIndicatorValue = int(qRound(vcm.volume / 2));
	if(indicatorValue_ != newIndicatorValue) {
		indicatorValue_ = newIndicatorValue;
		feedbackData.insert("indicator", indicatorValue_);
	}

	{
		QString newValue;
		if(vcm.isMuted)
			newValue = I18n::tr("title.muted");
		else if(vcmp) {
			newValue = QStringLiteral("%1 %").arg(qRound(vcm.volume));
			if(isSpeaking)
				newValue = QStringLiteral("\U0001F3A4 ") + newValue;
		}

		if(feedbackValue_ != newValue) {
			feedbackValue_ = newValue;
			feedbackData.insert("value", newValue);
		}
	}

	setFeedback(feedbackData);
}

void Action_VCMInfo::onPressed() {
	executeAction(Action(setting("pressAction").toInt()));
}

void Action_VCMInfo::onTapped() {
	executeAction(Action(setting("tapAction").toInt()));
}

void Action_VCMInfo::onReleased() {
	// Force state update (because pressing switches it)
	setState(state_);
}

void Action_VCMInfo::onRotated(int delta) {
	const auto vcmp = voiceChannelMember();
	if(!vcmp)
		return;

	const int stepSize = plugin()->globalSetting("voiceChannelVolumeEncoderStep").toInt();
	plugin()->adjustVoiceChannelMemberVolume(*vcmp.mem, stepSize, delta);
}

void Action_VCMInfo::executeAction(Action_VCMInfo::Action a) {
	switch(a) {

		case Action::muteUnmute: {
			const auto vcmp = voiceChannelMember();
			if(!vcmp)
				return;

			auto vcm = vcmp.mem;

			vcm->isMuted ^= true;

			plugin()->discord.sendCommand(+QDiscord::CommandType::setUserVoiceSettings, QJsonObject{
				{"user_id", vcm->userID},
				{"mute",    vcm->isMuted},
			});
			emit plugin()->buttonsUpdateRequested();
			break;
		}

		case Action::nextUser:
			if(plugin()->voiceChannelMembers.isEmpty())
				break;

			device()->voiceChannelMemberIndexOffset = (device()->voiceChannelMemberIndexOffset + 1) % plugin()->voiceChannelMembers.size();
			emit plugin()->buttonsUpdateRequested();
			break;

		case Action::previousUser:
			if(plugin()->voiceChannelMembers.isEmpty())
				break;

			device()->voiceChannelMemberIndexOffset = (device()->voiceChannelMemberIndexOffset + plugin()->voiceChannelMembers.size() - 1) % plugin()->voiceChannelMembers.size();
			emit plugin()->buttonsUpdateRequested();
			break;

		case Action::none:
			break;

	}
}

void Action_VCMInfo::buildPropertyInspector(QStreamDeckPropertyInspectorBuilder &b) {
	b.addCheckBox("hideNobodyInVoiceChatText", I18n::tr("pi.hideNobody")).linkWithGlobalSetting();

	// Order has to match the Action enum
	const QStringList actionSettings{
		I18n::tr("pi.action.muteUser"),
		I18n::tr("pi.action.nextUser"),
		I18n::tr("pi.action.previousUser"),
		I18n::tr("pi.action.none"),
	};
	b.addComboBox("pressAction", I18n::tr("pi.pressAction"), actionSettings).linkWithActionSetting();

	if(controller() == Controller::encoder) {
		b.addComboBox("tapAction", I18n::tr("pi.tapAction"), actionSettings).linkWithActionSetting();
		b.addCheckBox("showPaging", I18n::tr("pi.showPaging")).linkWithActionSetting();

		b.addSpinBox("voiceChannelVolumeEncoderStep", I18n::tr("pi.volumeStep")).linkWithGlobalSetting();
		b.addMessage(I18n::tr("pi.volumeStepDialsNote"));
	}


	VoiceChannelMemberAction::buildPropertyInspector(b);
}