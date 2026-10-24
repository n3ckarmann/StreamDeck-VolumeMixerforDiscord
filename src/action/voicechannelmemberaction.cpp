#include "voicechannelmemberaction.h"

#include <qtstreamdeck2/qstreamdeckpropertyinspectorbuilder.h>

#include "dvmplugin.h"
#include "i18n.h"
#include "dvmdevice.h"

VoiceChannelMemberAction::VoiceChannelMemberAction() {
	connect(this, &QStreamDeckAction::initialized, this, &VoiceChannelMemberAction::onInitialized);
}

void VoiceChannelMemberAction::onInitialized() {
	// Convert from string to int (legacy reasons)
	if(const auto v = setting("user_ix"); v.isDouble())
		setSetting("user_ix", QString::number(v.toInt()));
}

VoiceChannelMemberAction::VoiceChannelMemberResult VoiceChannelMemberAction::voiceChannelMember() {
	auto &lst = plugin()->voiceChannelMembers;

	QString userID = setting("user_ix").toString();

	VoiceChannelMemberResult r;

	// If the ID length is too low, that means that it's index in the list, not the actual user ID
	if(userID.length() < 4) {
		r.userIndex = userID.toInt() + device()->voiceChannelMemberIndexOffset;
		userID = lst.keys().value(r.userIndex);
	}

	auto it = lst.find(userID);
	r.mem = it != lst.end() ? &*it : nullptr;
	return r;
}

void VoiceChannelMemberAction::buildPropertyInspector(QStreamDeckPropertyInspectorBuilder &b) {
	b.addLineEdit("user_ix", I18n::tr("pi.userIndex")).linkWithActionSetting();
	b.addMessage(QStringList{
		I18n::tr("pi.userIndexHelp1"),
		I18n::tr("pi.userIndexHelp2"),
	});

	DVMAction::buildPropertyInspector(b);
}
