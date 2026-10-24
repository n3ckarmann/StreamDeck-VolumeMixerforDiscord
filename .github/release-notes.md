**[Volume Mixer for Discord](https://github.com/n3ckarmann/StreamDeck-VolumeMixerforDiscord)** continues Danol's Discord Volume Mixer, whose development has ended. Many thanks to Danol for the original! This fork is developed with the help of Claude Code. Setup guide, issues and source code: [github.com/n3ckarmann/StreamDeck-VolumeMixerforDiscord](https://github.com/n3ckarmann/StreamDeck-VolumeMixerforDiscord)

### Download

`com.n3ckarmann.volumemixer.streamDeckPlugin` below works on **Windows 10/11 and macOS 12+** (Apple Silicon and Intel). Double-click it to install. Requires **Stream Deck 7.6** or newer.

Coming from the old plugin? Install this one while the old one is still installed: your Client ID, secret and Discord login are taken over. Then remove the old plugin.

### Fixed in 2.1.2

- Stream Deck Neo: the bundled profile now puts "Who is speaking" on the infobar (it stayed black before)

### Fixed in 2.1.1

- The plugin can be installed on Windows again (2.1.0 stopped with an extraction error because of the macOS part of the file)

### New since the original (2.0.x)

_Setting up the Discord app is explained step by step in the [README](https://github.com/n3ckarmann/StreamDeck-VolumeMixerforDiscord#setting-up-the-discord-app)._

**New devices and systems**
- 🍎 macOS support
- Stream Deck **Neo** profile, plus a "Who is speaking" action for the Neo infobar (shows the name and avatar of the person speaking)
- Stream Deck **+ XL** profile with 6 dials

**Connection fixes**
- No more hanging on first use, keys no longer need to be placed again after a restart
- The Discord authorization dialog no longer pops up over and over, you have enough time to confirm it
- The login is kept when the internet or Discord is not available at boot, the plugin reconnects quietly as soon as Discord runs
- Keys show the connection status (e.g. "Discord off") instead of just doing nothing
- Mute keys no longer flicker when pressed

**Look and comfort**
- New icon set and app icon
- Client ID and secret are hidden behind an eye button
- Available in 11 languages: English, Deutsch, Español, Français, Italiano, Português, Polski, Nederlands, 日本語, 한국어, 简体中文

**Under the hood**
- Qt 6.8 LTS, no extra runtime or OpenSSL DLLs needed anymore
- Built automatically with GitHub Actions

### Note for macOS

The plugin is not notarized by Apple. If macOS blocks it, run this once in Terminal and restart Stream Deck:
```
xattr -dr com.apple.quarantine ~/Library/Application\ Support/com.elgato.StreamDeck/Plugins/com.n3ckarmann.volumemixer.sdPlugin
```

---

### Neu seit dem Original (Kurzfassung auf Deutsch)

- 2.1.2: Im Neo-Profil ist „Wer spricht“ jetzt direkt auf der Infobar (sie blieb vorher schwarz)
- 2.1.1: Das Plugin lässt sich unter Windows wieder installieren (2.1.0 brach beim Entpacken ab)

- macOS-Unterstützung, Profile für Stream Deck **Neo** (mit Infobar „Wer spricht“) und **+ XL**
- Verbindung zu Discord repariert: kein Hängen mehr, keine Dauer-Abfrage zum Autorisieren, Login bleibt beim Start ohne Internet erhalten, Tasten zeigen den Verbindungsstatus
- Neue Icons, Client ID/Secret versteckt, 11 Sprachen, keine zusätzlichen DLLs mehr
- Benötigt Stream Deck 7.6. Einstellungen des alten Plugins werden übernommen, wenn es beim Installieren noch da ist.
