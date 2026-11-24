#!/usr/bin/env bash
# Usage: scripts/package.sh <platform-name> <path-to-built-game-binary>
# Produces dist/<platform-name>/ containing the binary, runtime assets, run script,
# README, licences and BUILD_INFO. Fails (non-zero) on anything missing.
set -euo pipefail
NAME="$1"; BIN="$2"
SHA="${GITHUB_SHA:-$(git rev-parse HEAD)}"
OUT="dist/$NAME"
rm -rf "$OUT"; mkdir -p "$OUT"
[ -x "$BIN" ] || { echo "binary $BIN missing"; exit 1; }
cp "$BIN" "$OUT/game"
cp -R assets "$OUT/assets"
cp LICENSE "$OUT/LICENSE"
[ -f assets/credits/ASSET_LICENSES.md ] && cp assets/credits/ASSET_LICENSES.md "$OUT/ASSET_LICENSES.md" || echo "WARNING: no asset licence manifest yet"
cat > "$OUT/run.sh" <<'RUN'
#!/usr/bin/env sh
cd "$(dirname "$0")" && exec ./game "$@"
RUN
chmod +x "$OUT/run.sh" "$OUT/game"
cat > "$OUT/README.txt" <<TXT
game.izdrail.com ($NAME)
Run:  ./run.sh        (must be started from this folder or via run.sh)
Linux needs: libglfw3, libassimp, libfreetype6, an OpenGL 3.3 capable GPU/driver.
macOS: brew install glfw assimp freetype
TXT
printf 'commit: %s\nplatform: %s\nbuilt: %s\nrun: %s\n' "$SHA" "$NAME" "$(date -u +%FT%TZ)" "${GITHUB_RUN_ID:-local}" > "$OUT/BUILD_INFO.txt"
( cd dist && tar -czf "$NAME.tar.gz" "$NAME" )
echo "packaged dist/$NAME.tar.gz"
