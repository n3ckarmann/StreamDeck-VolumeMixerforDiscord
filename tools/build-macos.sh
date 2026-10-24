#!/usr/bin/env bash
# Builds the plugin and packages it as a .streamDeckPlugin file (macOS, Qt 6.8, universal binary)
#
#   ./tools/build-macos.sh /path/to/Qt/6.8.3/macos
#
# The result is written to release/com.n3ckarmann.volumemixer.streamDeckPlugin.
# Set CODESIGN_IDENTITY to a "Developer ID Application" identity to sign properly; by default the bundle is ad-hoc signed.

set -euo pipefail

qt_dir="${1:?Usage: $0 <Qt dir>}"
root="$(cd "$(dirname "$0")/.." && pwd)"
build_dir="$root/build"
plugin_dir="$root/bin/Release/com.n3ckarmann.volumemixer.sdPlugin"
release_dir="$root/release"
bundle="$plugin_dir/bin/streamdeck-volumemixer.app"

rm -rf "$build_dir" "$plugin_dir" "$release_dir"

echo "==> Configure"
cmake -S "$root" -B "$build_dir" -G Ninja -DCMAKE_BUILD_TYPE=Release "-DCMAKE_PREFIX_PATH=$qt_dir"
echo "==> Build"
cmake --build "$build_dir" --parallel
echo "==> Install + macdeployqt"
cmake --install "$build_dir"

# macdeployqt rewrites the binaries, which breaks their signatures. Apple Silicon refuses to run unsigned code,
# so the whole bundle is signed again (ad-hoc unless an identity is given).
echo "==> Sign"
if [[ -n "${CODESIGN_IDENTITY:-}" ]]; then
    codesign --force --deep --options runtime --timestamp --sign "$CODESIGN_IDENTITY" "$bundle"
else
    codesign --force --deep --sign - "$bundle"
fi
codesign --verify --deep --strict "$bundle"
lipo -info "$bundle/Contents/MacOS/streamdeck-volumemixer"

# Stream Deck expects a zip with the .sdPlugin folder at its root (-y keeps the framework symlinks)
mkdir -p "$release_dir"
target="$release_dir/com.n3ckarmann.volumemixer.streamDeckPlugin"
(cd "$(dirname "$plugin_dir")" && zip -qry "$target" "$(basename "$plugin_dir")" -x '*.DS_Store')

echo "Done: $target"
