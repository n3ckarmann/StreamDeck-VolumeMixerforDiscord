#pragma once

#include "dvmaction.h"

class Action_OpenMixer : public DVMAction {
Q_OBJECT

public:
	Action_OpenMixer();

public:
	virtual void update() override;

private slots:
	void onPressed();

};
