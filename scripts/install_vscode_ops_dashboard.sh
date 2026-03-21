#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
project_root="$(cd "$script_dir/.." && pwd)"
extension_src="$project_root/tools/vscode-war-ops-dashboard"
extension_name="imperator-local.war-ops-dashboard-0.0.1"

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

mkdir -p "$target_base"
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
echo "5. Use tools/vscode-war-ops-dashboard/README.md if you need the local guide."
