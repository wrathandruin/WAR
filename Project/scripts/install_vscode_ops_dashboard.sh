#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "$script_dir/../.." && pwd)"
extension_src="$repo_root/Tools/vscode-war-ops-dashboard"
extension_name="imperator-local.war-ops-dashboard-0.0.3"
extension_id="imperator-local.war-ops-dashboard"

if [ ! -f "$extension_src/package.json" ]; then
  echo "ERROR: Extension source not found at $extension_src" >&2
  exit 1
fi

target_base="${1:-}"

if [ -z "$target_base" ]; then
  if [ -d "$HOME/.vscode-server/extensions" ]; then
    target_base="$HOME/.vscode-server/extensions"
  else
    target_base="$HOME/.vscode/extensions"
  fi
fi

target_dir="$target_base/$extension_name"
extensions_registry="$target_base/extensions.json"
profile_cache="$HOME/.vscode-server/data/CachedProfilesData/__default__profile__/extensions.user.cache"

mkdir -p "$target_base"

for existing_dir in "$target_base"/imperator-local.war-ops-dashboard-*; do
  if [ ! -e "$existing_dir" ]; then
    continue
  fi

  if [ "$existing_dir" != "$target_dir" ]; then
    rm -rf "$existing_dir"
  fi
done

for stale_dir in "$target_base"/imperator-local.swr-ops-panel-*; do
  if [ ! -e "$stale_dir" ]; then
    continue
  fi

  rm -rf "$stale_dir"
done

if [ -f "$extensions_registry" ]; then
  python3 - "$extensions_registry" "$target_dir" "$extension_name" "$extension_id" "$extension_src/package.json" <<'PY'
import json
import sys

path = sys.argv[1]
target_dir = sys.argv[2]
extension_name = sys.argv[3]
extension_id = sys.argv[4]
package_json_path = sys.argv[5]

with open(package_json_path, "r", encoding="utf-8") as handle:
    package = json.load(handle)

with open(path, "r", encoding="utf-8") as handle:
    data = json.load(handle)

filtered = []
for item in data:
    identifier = ((item or {}).get("identifier") or {}).get("id", "")
    if identifier in {"imperator-local.swr-ops-panel", extension_id}:
        continue
    filtered.append(item)

filtered.append(
    {
        "identifier": {"id": extension_id},
        "version": package["version"],
        "location": {"$mid": 1, "path": target_dir, "scheme": "file"},
        "relativeLocation": extension_name,
    }
)

with open(path, "w", encoding="utf-8") as handle:
    json.dump(filtered, handle, separators=(",", ":"))
PY
fi

rm -f "$profile_cache"

rm -rf "$target_dir"
cp -R "$extension_src" "$target_dir"

echo "Installed WAR Ops: Dashboard to:"
echo "$target_dir"
echo
echo "Next steps:"
echo "1. Reload VS Code."
echo "2. Open the WAR Ops activity-bar view."
echo "3. Or run: WAR Ops: Dashboard: Open Dashboard from the Command Palette."
echo "4. The sticky status strip should appear at the top of the dashboard."
echo "5. Use Tools/vscode-war-ops-dashboard/README.md if you need the local guide."
