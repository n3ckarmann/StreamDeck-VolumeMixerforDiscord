<div align="center">

<img src="dist/icons/app_large@2x.png" width="128" alt="Volume Mixer for Discord icon">

# Volume Mixer for Discord – Stream Deck plugin

**Change the volume of every person in your Discord voice chat, right from your Stream Deck.**

English · [Deutsch](README.de.md)

</div>

---

## What it does

- 🔊 **Volume per person** – turn people in your voice channel up or down, perfect with the dials of the **Stream Deck +**
- 🔇 **Mute anyone** with a single press
- 🟢 **See who is speaking** – the key lights up while someone talks
- 🎙️ **Your own microphone and deafen** on a key
- 🧩 **Works on all decks** – Standard, Mini, XL, Mobile, Neo, Stream Deck + and Stream Deck + XL, on Windows and macOS
- ℹ️ **Neo infobar** shows who is speaking right now
- 🌍 **Available in** English, Deutsch, Español, Français, Italiano, Português, Polski, Nederlands, 日本語, 한국어 and 简体中文

![Screenshot](etc/sshot.png)

## Installation

1. Download the latest `.streamDeckPlugin` file from the [releases](../../releases) and double-click it.
2. Put the **Volume Mixer** action on a key.
3. Set up your own Discord app (once, takes two minutes) – see below.

> **Coming from Danol's original Discord Volume Mixer?** Install this plugin **before** removing the old one: on its first start it takes over your Client ID, secret and Discord login, so you don't have to set anything up again. Then uninstall the old plugin and put the keys on your Stream Deck again (Stream Deck treats this as a new plugin).

## Setting up the Discord app

Discord only lets apps that you registered yourself control your voice chat, so every user needs their own (free) Discord app:

1. Open the [Discord Developer Portal](https://discord.com/developers/applications) with **the same account you use in the Discord app** and click **New Application**.
   - Pick any name, but **don't use the word "Discord"** in it (Discord rejects that).
2. Under **OAuth2**, add the redirect `http://localhost:1337/callback` and click **Save Changes**.
3. Copy the **Client ID** and the **Client Secret** (click *Reset Secret* if you only see that button).
4. Paste both into the settings of the Volume Mixer key on your Stream Deck. They are hidden – click the 👁 eye to show them.
5. Press the key and click **Authorize** in the window Discord shows you. Done!

Treat the Client Secret like a password and never share it.

![OAuth2 settings](etc/oauth.png)

## What the keys tell you

| Text on the key | Meaning | What to do |
| :--- | :--- | :--- |
| `SETUP NEEDED` | Client ID or secret is missing | Fill in both in the key settings |
| `DISCORD NOT RUNNING` | The Discord app is closed | Start Discord – the plugin connects by itself |
| `DISCORD NOT READY` | Discord is still starting | Wait a moment |
| `AUTHORIZE IN DISCORD` | Discord shows an authorization window | Click **Authorize** in Discord |
| `DECLINED – PRESS TO RETRY` | The authorization was declined or timed out | Press the key and authorize in Discord |
| `CHECK CLIENT ID` | Discord doesn't accept the Client ID | Check the ID; the app must belong to the account you use in Discord |
| `CHECK SECRET` | Discord doesn't accept the secret | Reset the secret in the Developer Portal and paste the new one |
| `WAITING FOR INTERNET` | Discord's servers can't be reached (e.g. right after booting) | Nothing – it reconnects automatically |
| `NOBODY IN VOICE` | You are alone or not in a voice channel | Join a voice channel |

The **Volume Mixer**, **Microphone** and **Deafen** keys are dimmed while the plugin is not connected and show a short status: red `DISCORD OFF` when Discord is closed, yellow while connecting or waiting for you to authorize. As soon as you start Discord, the plugin connects within a few seconds.

Also make sure Discord and the Stream Deck software both run **without** administrator rights.

### Sending a log

Create an empty file `clog.txt` in `%appdata%\Elgato\StreamDeck\Plugins\com.n3ckarmann.volumemixer.sdPlugin\` (macOS: `~/Library/Application Support/com.elgato.StreamDeck/Plugins/com.n3ckarmann.volumemixer.sdPlugin/`), reproduce the problem and attach the file to an [issue](../../issues). Delete it afterwards, it keeps growing.

## Building from source

Requirements: Windows 10/11 x64, [Qt 6.8 LTS](https://www.qt.io/download-qt-installer) with the *Qt WebSockets* module, CMake 3.20+ and either MinGW 13 or MSVC 2022.

```powershell
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=C:\Qt\6.8.3\mingw_64
cmake --build build
cmake --install build      # copies everything to bin\Release\com.n3ckarmann.volumemixer.sdPlugin and runs windeployqt
```

With `-DBUILD_IN_STREAMDECK_PLUGINS_DIR=ON` the plugin is built straight into your Stream Deck plugins folder.

To get an installable `.streamDeckPlugin` file in one go: `.\tools\build-windows.ps1 -QtDir C:\Qt\6.8.3\mingw_64 -MinGWDir C:\Qt\Tools\mingw1310_64`

On macOS (12+, Xcode command line tools, Ninja, Qt 6.8 for macOS) `./tools/build-macos.sh ~/Qt/6.8.3/macos` builds a universal binary (Apple Silicon and Intel) and packages it.

- Texts and translations: [`src/translations.json`](src/translations.json) (button titles and settings) and `dist/<language>.json` (action names in the Stream Deck app)
- Icons: SVG sources in [`etc/icons`](etc/icons)

## How this fork is made 🤖

Full transparency: this fork is **vibe-coded**. The maintainer describes what should change and tests everything on real hardware, while the code, icons and translations are written by an AI coding assistant ([Claude Code](https://claude.com/claude-code)). Every change is still reviewed, built and tested before a release, and the whole history is public in the commits.

If you find something odd, please open an [issue](../../issues) – bug reports are very welcome.

## Credits

This plugin was created by **[Danol (Daniel Čejchan)](https://github.com/CZDanol)** – the [original project](https://github.com/CZDanol/StreamDeck-DiscordVolumeMixer2), including the [QtStreamDeck2](https://github.com/CZDanol/QtStreamDeck2) and [QtDiscordIPC](https://github.com/CZDanol/QtDiscordIPC) libraries it is built on, is his work. Since he ended development in 2024, the plugin is continued here as a community fork. Thank you, Danol!

- **Danol** – original author
- **[Krabs](https://github.com/krabs-github)** – profiles for the XL and testing of the original plugin
- **n3ckarmann** – maintainer of this fork (vibe-coded with Claude Code)
- Built with [Qt 6](https://www.qt.io/) (LGPLv3)

Icons and app icon were made for this fork and are part of the project (GPLv3). This project is not affiliated with Discord; "for Discord" only describes what it works with.

## License

[GNU General Public License v3.0](LICENSE.md), like the original project.

<sub>Not affiliated with or endorsed by Discord Inc. or Elgato/Corsair. Discord is a trademark of Discord Inc.</sub>
