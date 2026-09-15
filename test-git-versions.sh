#!/usr/bin/env bash
set -euo pipefail

root=$(mktemp -d)
trap 'rm -rf "$root"' EXIT

git init -q "$root/library"
git -C "$root/library" config user.name Test
git -C "$root/library" config user.email test@example.com
mkdir -p "$root/library/fixture"
printf '#pragma once\n' > "$root/library/fixture/fixture.hpp"
git -C "$root/library" add fixture/fixture.hpp
git -C "$root/library" commit -qm 'Add fixture header'
revision=$(git -C "$root/library" rev-parse HEAD)
uppercaseRevision=$(printf '%s' "$revision" | tr '[:lower:]' '[:upper:]')

mkdir -p "$root/project/repository"
printf '#include <fixture/fixture.hpp>\n' > "$root/project/main.cpp"
printf 'remoteRepository=""\nlocalRepository="repository"\n' > "$root/project/coddle.toml"
printf '[[library]]\ntype="git"\nname="fixture"\npath="%s"\nversion="%s"\nincludes=["fixture/fixture.hpp"]\n' \
  "$root/library" "$uppercaseRevision" > "$root/project/repository/libraries.toml"

(cd "$root/project" && "$OLDPWD/coddle" dep-only)
(cd "$root/project" && "$OLDPWD/coddle" dep-only)
test "$(git -C "$root/project/.coddle/libs_src/fixture" rev-parse HEAD)" = "$revision"
