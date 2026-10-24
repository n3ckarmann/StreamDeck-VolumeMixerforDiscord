#pragma once

#include "dvmaction.h"

/// Stream Deck Neo infobar: shows who in the voice channel is speaking right now
class Action_SpeakingInfo : public DVMAction {
Q_OBJECT

public:
	Action_SpeakingInfo();

public slots:
	virtual void update() override;

private slots:
	void onAppeared();

private:
	QString title_ = "###";
	QString value_ = "###";
	QString iconUserID_ = "###";
	bool hasAvatar_ = false;

};
