#include "i18n.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

namespace {
	QJsonObject strings;
	QString language = QStringLiteral("en");

	void load() {
		if(!strings.isEmpty())
			return;

		QFile f(":/translations.json");
		if(!f.open(QIODevice::ReadOnly)) {
			qWarning() << "I18n: translations not found";
			return;
		}

		strings = QJsonDocument::fromJson(f.readAll()).object()["strings"].toObject();
	}
}

void I18n::setLanguage(const QString &lang) {
	load();

	// Use the exact language if available ("zh_CN"), otherwise the base language ("pt_BR" -> "pt")
	const QString base = lang.section('_', 0, 0);
	const QJsonObject sample = strings.begin() != strings.end() ? strings.begin()->toObject() : QJsonObject();

	if(sample.contains(lang))
		language = lang;
	else if(sample.contains(base))
		language = base;
	else
		language = QStringLiteral("en");

	qDebug() << "I18n: language" << lang << "->" << language;
}

QString I18n::tr(const char *key) {
	load();

	const QJsonObject entry = strings[QLatin1String(key)].toObject();
	if(entry.isEmpty()) {
		qWarning() << "I18n: missing key" << key;
		return QString::fromLatin1(key);
	}

	const QString text = entry[language].toString();
	return text.isEmpty() ? entry["en"].toString() : text;
}
