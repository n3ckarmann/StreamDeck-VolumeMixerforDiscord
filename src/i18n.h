#pragma once

#include <QString>

/// Translations of the plugin's own texts (property inspector, button titles).
/// The texts live in translations.json, which is compiled into the executable.
namespace I18n {

	/// Selects the language, e.g. "de", "pt_BR" or "zh_CN". Unknown languages fall back to English.
	void setLanguage(const QString &language);

	/// Returns the text for key in the selected language (or English if there is no translation)
	QString tr(const char *key);

}
