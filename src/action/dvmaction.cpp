#include "dvmaction.h"

#include <qtstreamdeck2/qstreamdeckpropertyinspectorbuilder.h>

#include "dvmplugin.h"
#include "i18n.h"

DVMAction::DVMAction() {
	connect(this, &QStreamDeckAction::initialized, this, &DVMAction::onInitialized);
	connect(this, &QStreamDeckAction::settingsChanged, this, &DVMAction::update);
}

void DVMAction::buildPropertyInspector(QStreamDeckPropertyInspectorBuilder &b) {
	const auto link = [](const QString &url, const QString &text) {
		return QStringLiteral("<a href=\"javascript: openUrl('%1');\">%2</a>").arg(url, text);
	};

	b.addSection(I18n::tr("pi.section.discord"));
	b.addSecretLineEdit("client_id", I18n::tr("pi.clientId")).setToggleLabels(I18n::tr("pi.show"), I18n::tr("pi.hide")).linkWithGlobalSetting();
	b.addSecretLineEdit("client_secret", I18n::tr("pi.clientSecret")).setToggleLabels(I18n::tr("pi.show"), I18n::tr("pi.hide")).linkWithGlobalSetting();
	b.addMessage(QStringList{
		I18n::tr("pi.setupHelp").arg(link(DVMPlugin::setupGuideUrl, I18n::tr("pi.setupGuide"))),
		I18n::tr("pi.secretNote"),
	});
	b.addHTML(QStringLiteral("<div class=\"sdpi-item\"><div class=\"sdpi-item-label empty\"></div><div class=\"sdpi-item-value credits\">%1</div></div>")
		.arg(I18n::tr("pi.credits").arg(link(DVMPlugin::originalProjectUrl, QStringLiteral("Danol (CZDanol)")))));
}

void DVMAction::onInitialized() {
	connect(plugin(), &DVMPlugin::buttonsUpdateRequested, this, &DVMAction::update);
	connect(plugin(), &DVMPlugin::globalSettingsChanged, this, &DVMAction::update);

	QTimer::singleShot(0, this, &DVMAction::update);
}