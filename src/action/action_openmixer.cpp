#include "action_openmixer.h"

#include "dvmplugin.h"

Action_OpenMixer::Action_OpenMixer() {
	connect(this, &QStreamDeckAction::keyDown, this, &Action_OpenMixer::onPressed);
}

void Action_OpenMixer::update() {
	updateConnectionBadge({"app"});
}

void Action_OpenMixer::onPressed() {
	using DT = QStreamDeckDevice::DeviceType;
	static const QMap<int, QString> profileNameByDeviceType{
		{+DT::streamDeck,       "Volume Mixer for Discord"},
		{+DT::streamDeckMini,   "Volume Mixer for Discord Mini"},
		{+DT::streamDeckXL,     "Volume Mixer for Discord XL"},
		{+DT::streamDeckMobile, "Volume Mixer for Discord"},
		{+DT::streamDeckPlus,   "Volume Mixer for Discord+"},
	};

	device()->switchToProfile(profileNameByDeviceType.value(+device()->deviceType(), "Volume Mixer for Discord"));
	plugin()->updateChannelMembersData();
}
