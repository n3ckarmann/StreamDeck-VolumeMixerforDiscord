#pragma once

#include <qtstreamdeck2/qstreamdeckaction.h>

#include "dvmdevice.h"

class DVMAction : public QStreamDeckActionT<DVMDevice> {
Q_OBJECT

public:
	DVMAction();

public slots:
	virtual void update() {};

protected:
	virtual void buildPropertyInspector(QStreamDeckPropertyInspectorBuilder &b) override;

	/// While Discord is not connected, dims the key icons (one per state) and shows the connection status on them
	void updateConnectionBadge(const QStringList &stateIcons);

private slots:
	void onInitialized();

private:
	QString badge_ = QStringLiteral("?");

};