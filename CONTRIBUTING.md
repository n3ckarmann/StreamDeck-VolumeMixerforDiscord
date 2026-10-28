# How bugs and releases are handled

**English** · [Deutsch](#deutsch)

1. **Every bug gets an issue.** As soon as a bug is found, whether by a user or while testing, it is filed as an issue in the [public repository](https://github.com/n3ckarmann/StreamDeck-VolumeMixerforDiscord/issues): what is affected, the cause once known, a workaround if there is one, and the planned fix.
2. **Bugs are fixed in a new version.** A released version is never changed afterwards. The fix raises the version in `dist/manifest.json` and ships as the next release.
3. **Every release has release notes.** `.github/release-notes.md` lists what changed in the new version and links the issues it fixes.
4. **The fix closes the issue.** The commit that brings the fix to the public repository says `Fixes #N`, and the issue gets a short comment naming the version that contains the fix.
5. **Test before release.** Each release is tested on a real Stream Deck before it goes out. Release builds come only from GitHub Actions.

Found a bug? Please [open an issue](https://github.com/n3ckarmann/StreamDeck-VolumeMixerforDiscord/issues/new) and attach `clog.txt` if you can (see the README). Never paste your Discord Client Secret or login token into an issue.

## Deutsch

1. **Jeder Fehler wird ein Issue.** Sobald ein Fehler auffällt, ob von Nutzern gemeldet oder beim Testen gefunden, wird er als Issue im [öffentlichen Repository](https://github.com/n3ckarmann/StreamDeck-VolumeMixerforDiscord/issues) angelegt: was betroffen ist, die Ursache, sobald sie bekannt ist, ein Workaround, falls es einen gibt, und der geplante Fix.
2. **Fehler werden in einer neuen Version behoben.** Eine veröffentlichte Version wird nachträglich nicht geändert. Der Fix erhöht die Version in `dist/manifest.json` und kommt mit dem nächsten Release.
3. **Jedes Release hat Release Notes.** `.github/release-notes.md` nennt die Änderungen der neuen Version und verlinkt die behobenen Issues.
4. **Der Fix schließt das Issue.** Der Commit, der den Fix ins öffentliche Repository bringt, enthält `Fixes #N`, und das Issue bekommt einen kurzen Kommentar mit der Version, die den Fix enthält.
5. **Vor dem Release wird getestet.** Jedes Release wird vor der Veröffentlichung auf einem echten Stream Deck geprüft. Release-Builds entstehen nur in GitHub Actions.

Einen Fehler gefunden? Bitte [ein Issue anlegen](https://github.com/n3ckarmann/StreamDeck-VolumeMixerforDiscord/issues/new) und wenn möglich die `clog.txt` anhängen (siehe README). Niemals das Discord Client Secret oder den Login-Token in ein Issue kopieren.
