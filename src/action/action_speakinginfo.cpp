#include "action_speakinginfo.h"

#include <QPainter>
#include <QPainterPath>

#include "dvmplugin.h"
#include "i18n.h"

Action_SpeakingInfo::Action_SpeakingInfo() {
	connect(this, &QStreamDeckAction::initialized, this, &Action_SpeakingInfo::onAppeared);
}

void Action_SpeakingInfo::onAppeared() {
	// The infobar has no built-in layout, it has to be set every time the action appears
	setFeedbackLayout("layouts/speaking.json");
	title_ = value_ = iconUserID_ = "###";
	update();
}

void Action_SpeakingInfo::update() {
	DVMPlugin *p = plugin();

	QString newTitle, newValue, newIconUserID;
	if(!p->discord.isConnected()) {
		newTitle = I18n::tr("infobar.discord");
		newValue = QString(p->connectionStatusText()).replace('\n', ' ');
	}
	else if(p->currentVoiceChannelID.isEmpty()) {
		newTitle = I18n::tr("infobar.discord");
		newValue = I18n::tr("infobar.notInVoice");
	}
	else {
		QStringList speaking;
		for(const VoiceChannelMember &vcm: p->voiceChannelMembers) {
			if(!p->speakingVoiceChannelMembers.contains(vcm.userID))
				continue;

			if(newIconUserID.isEmpty())
				newIconUserID = vcm.userID;

			speaking << vcm.nick;
		}

		newTitle = I18n::tr("infobar.speaking");
		newValue = speaking.isEmpty() ? QStringLiteral("–") : speaking.join(", ");
	}

	QJsonObject feedback;
	if(newTitle != title_)
		feedback.insert("title", title_ = newTitle);

	if(newValue != value_)
		feedback.insert("value", value_ = newValue);

	if(newIconUserID != iconUserID_ || (!newIconUserID.isEmpty() && !hasAvatar_)) {
		iconUserID_ = newIconUserID;

		QImage img(64, 64, QImage::Format_ARGB32_Premultiplied);
		img.fill(Qt::transparent);
		QPainter painter(&img);
		painter.setRenderHint(QPainter::Antialiasing);
		painter.setRenderHint(QPainter::SmoothPixmapTransform);

		// Avatar of the first person speaking, otherwise the microphone icon
		const QImage avatar = iconUserID_.isEmpty() ? QImage() : p->discord.getUserAvatar(iconUserID_, p->voiceChannelMembers.value(iconUserID_).avatarID);
		hasAvatar_ = !avatar.isNull();
		if(hasAvatar_) {
			QPainterPath clip;
			clip.addEllipse(img.rect());
			painter.setClipPath(clip);
			painter.drawImage(img.rect(), avatar);
		}
		else {
			static const QImage micIcon("icons/mic_on@2x.png");
			painter.drawImage(img.rect(), micIcon);
		}

		feedback.insert("icon", QStreamDeckPlugin::encodeImage(img));
	}

	if(!feedback.isEmpty())
		setFeedback(feedback);
}
