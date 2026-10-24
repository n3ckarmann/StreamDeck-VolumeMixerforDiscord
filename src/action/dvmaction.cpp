#include "dvmaction.h"

#include <QPainter>

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
void DVMAction::updateConnectionBadge(const QStringList &stateIcons) {
	enum class Severity { warning, error };
	struct Badge {
		const char *key;
		Severity severity;
	};
	static const QHash<QString, Badge> badges{
		{"CONNECTING", {"badge.connecting", Severity::warning}},
		{"AUTHORIZE",  {"badge.authorize", Severity::warning}},
		{"ERR 8",      {"badge.connecting", Severity::warning}},
		{"ERR 3",      {"badge.noInternet", Severity::warning}},
		{"ERR 1",      {"badge.offline", Severity::error}},
	};

	const QDiscord &discord = plugin()->discord;
	const Badge badge = discord.isConnected() ? Badge{nullptr, Severity::error} : badges.value(discord.connectionError(), Badge{"badge.notConnected", Severity::error});
	const QString text = badge.key ? I18n::tr(badge.key) : QString();

	if(badge_ == text)
		return;

	badge_ = text;

	for(int state = 0; state < stateIcons.size(); state++) {
		QImage img(QStringLiteral("icons/%1@2x.png").arg(stateIcons[state]));
		if(img.isNull())
			continue;

		img = img.convertToFormat(QImage::Format_ARGB32_Premultiplied);
		img.setDevicePixelRatio(1); // Qt picks up the @2x from the file name, but we paint in pixels

		if(!text.isEmpty()) {
			const int w = img.width(), h = img.height();
			QPainter p(&img);
			p.setRenderHint(QPainter::Antialiasing);
			p.setRenderHint(QPainter::TextAntialiasing);

			// Dim the icon
			p.fillRect(img.rect(), QColor(0, 0, 0, 150));

			// Status strip at the bottom
			const QRectF strip(w * 0.06, h * 0.68, w * 0.88, h * 0.24);
			p.setPen(Qt::NoPen);
			p.setBrush(badge.severity == Severity::error ? QColor(0xE5, 0x3E, 0x3E) : QColor(0xF0, 0xA0, 0x20));
			p.drawRoundedRect(strip, h * 0.06, h * 0.06);

			QFont font;
			font.setBold(true);
			font.setPixelSize(int(h * 0.13));
			while(font.pixelSize() > 8 && QFontMetrics(font).horizontalAdvance(text) > strip.width() - w * 0.06)
				font.setPixelSize(font.pixelSize() - 1);

			p.setFont(font);
			p.setPen(badge.severity == Severity::error ? Qt::white : QColor(0x20, 0x16, 0x00));
			p.drawText(strip, Qt::AlignCenter, text);
		}

		setImage(img, state);
	}
}
