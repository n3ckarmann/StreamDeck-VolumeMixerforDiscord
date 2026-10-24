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

# Stream Deck for Windows cannot extract symlinks, and the combined .streamDeckPlugin carries this bundle too.
# So the Qt frameworks (which need Versions/Current symlinks) are turned into plain dylibs in Contents/Frameworks.
echo "==> Flatten frameworks"
frameworks="$bundle/Contents/Frameworks"
machos=("$bundle/Contents/MacOS/streamdeck-volumemixer")
while IFS= read -r f; do machos+=("$f"); done < <(find "$bundle/Contents/PlugIns" -name '*.dylib')
for fw in "$frameworks"/*.framework; do
    name="$(basename "$fw" .framework)"
    mv "$fw/Versions/A/$name" "$frameworks/$name.dylib"
    rm -rf "$fw"
    install_name_tool -id "@rpath/$name.dylib" "$frameworks/$name.dylib" 2>/dev/null
    machos+=("$frameworks/$name.dylib")
done
for m in "${machos[@]}"; do
    otool -L "$m" | awk 'NR>1 && $1 ~ /^@rpath\/[^\/]+\.framework\// {print $1}' | while read -r dep; do
        name="$(basename "$dep")"
        install_name_tool -change "$dep" "@rpath/$name.dylib" "$m" 2>/dev/null
    done
done
if [[ -n "$(find "$bundle" -type l)" ]]; then
    echo "Symlinks left in the bundle:"; find "$bundle" -type l; exit 1
fi

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

# Smoke test: dyld must find every library (the plugin itself exits soon without Stream Deck arguments)
echo "==> Smoke test"
set +e
out="$(DYLD_PRINT_LIBRARIES=1 "$bundle/Contents/MacOS/streamdeck-volumemixer" 2>&1 & pid=$!; sleep 5; kill $pid 2>/dev/null; wait $pid)"
set -e
if grep -qE 'Library not loaded|image not found|Symbol not found' <<<"$out"; then
    echo "$out"; exit 1
fi
grep -q 'Frameworks/QtCore.dylib' <<<"$out" || { echo "$out"; echo "QtCore was not loaded from the bundle"; exit 1; }

# Stream Deck expects a zip with the .sdPlugin folder at its root
mkdir -p "$release_dir"
target="$release_dir/com.n3ckarmann.volumemixer.streamDeckPlugin"
(cd "$(dirname "$plugin_dir")" && zip -qr "$target" "$(basename "$plugin_dir")" -x '*.DS_Store')

echo "Done: $target"
