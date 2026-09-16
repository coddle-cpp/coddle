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

# Reuse the same cache after advancing an exact library pin.
binary="$PWD/coddle"
cache="$root/project/.coddle/libs_src/fixture"
run() { (cd "$root/project" && "$binary" dep-only); }
reject() {
  if run > "$root/failure.log" 2>&1; then
    echo 'Expected cache update to fail' >&2
    exit 1
  fi
}
set_library() {
  printf '[[library]]\ntype="git"\nname="fixture"\npath="%s"\nversion="%s"\nincludes=["fixture/fixture.hpp"]\npostClone="exit 99"\n' \
    "$1" "$2" > "$root/project/repository/libraries.toml"
}
printf '*.local\n' > "$root/library/.gitignore"
git -C "$root/library" add .gitignore
git -C "$root/library" commit -qm 'Ignore local scratch files'
old=$(git -C "$root/library" rev-parse HEAD)
set_library "$root/library" "$old"
run
test "$(git -C "$cache" rev-parse HEAD)" = "$old"
printf '#pragma once\n// new pin\n' > "$root/library/fixture/fixture.hpp"
printf 'upstream\n' > "$root/library/collision.local"
git -C "$root/library" add fixture/fixture.hpp
git -C "$root/library" add -f collision.local
git -C "$root/library" commit -qm 'Advance fixture'
new=$(git -C "$root/library" rev-parse HEAD)
set_library "$root/library" "$new"

# Tracked, staged and untracked changes all block repair without losing data.
printf '// local edit\n' >> "$cache/fixture/fixture.hpp"
cp "$cache/fixture/fixture.hpp" "$root/edited.hpp"
reject
cmp "$cache/fixture/fixture.hpp" "$root/edited.hpp"
git -C "$cache" add fixture/fixture.hpp
index=$(git -C "$cache" write-tree)
reject
test "$(git -C "$cache" write-tree)" = "$index"
cmp "$cache/fixture/fixture.hpp" "$root/edited.hpp"
git -C "$cache" restore --staged --worktree fixture/fixture.hpp
printf 'keep\n' > "$cache/untracked.txt"
reject
test "$(cat "$cache/untracked.txt")" = keep
rm "$cache/untracked.txt"
test "$(git -C "$cache" rev-parse HEAD)" = "$old"

# An ignored file must not be overwritten by a newly tracked target path.
printf 'keep ignored\n' > "$cache/collision.local"
reject
test "$(cat "$cache/collision.local")" = 'keep ignored'
test "$(git -C "$cache" rev-parse HEAD)" = "$old"
rm "$cache/collision.local"
run
test "$(git -C "$cache" rev-parse HEAD)" = "$new"
test "$(cat "$cache/collision.local")" = upstream
# A cached object needs no network or working origin; postClone must not rerun.
set_library "$root/missing-origin" "$old"
run
test "$(git -C "$cache" rev-parse HEAD)" = "$old"
set_library "$root/missing-origin" "$new"
run
test "$(git -C "$cache" rev-parse HEAD)" = "$new"

# Failed fetch leaves HEAD, index and files unchanged.
set_library "$root/missing-origin" 1111111111111111111111111111111111111111
head=$(git -C "$cache" rev-parse HEAD)
index=$(git -C "$cache" write-tree)
cp "$cache/fixture/fixture.hpp" "$root/before.hpp"
reject
test "$(git -C "$cache" rev-parse HEAD)" = "$head"
test "$(git -C "$cache" write-tree)" = "$index"
cmp "$cache/fixture/fixture.hpp" "$root/before.hpp"
test -z "$(git -C "$cache" status --porcelain)"

# Catalog cache pins follow the same update path; local entries still override.
git init -q "$root/catalog"
git -C "$root/catalog" config user.name Test
git -C "$root/catalog" config user.email test@example.com
printf '# catalog one\n' > "$root/catalog/libraries.toml"
git -C "$root/catalog" add libraries.toml
git -C "$root/catalog" commit -qm 'First catalog'
catalog_old=$(git -C "$root/catalog" rev-parse HEAD)
printf 'remoteRepository="%s"\nremoteVersion="%s"\nlocalRepository="repository"\n' \
  "$root/catalog" "$catalog_old" > "$root/project/coddle.toml"
set_library "$root/library" "$new"
run
printf '[[library]]\ntype="git"\nname="fixture"\npath="%s"\nversion="invalid-remote-version"\nincludes=["fixture/fixture.hpp"]\n' \
  "$root/missing-origin" > "$root/catalog/libraries.toml"
git -C "$root/catalog" commit -qam 'Second catalog'
catalog_new=$(git -C "$root/catalog" rev-parse HEAD)
printf 'remoteRepository="%s"\nremoteVersion="%s"\nlocalRepository="repository"\n' \
  "$root/catalog" "$catalog_new" > "$root/project/coddle.toml"
run
test "$(git -C "$root/project/.coddle/remote" rev-parse HEAD)" = "$catalog_new"
test "$(git -C "$cache" rev-parse HEAD)" = "$new"

# A directory inside a parent repository is not a dependency repository.
mv "$cache" "$root/saved-cache"
mkdir "$cache"
git -C "$root/project" init -q
git -C "$root/project" config user.name Test
git -C "$root/project" config user.email test@example.com
git -C "$root/project" add main.cpp
git -C "$root/project" commit -qm 'Parent repository'
parent_head=$(git -C "$root/project" rev-parse HEAD)
reject
grep -q 'cache must be a Git worktree root' "$root/failure.log"
test "$(git -C "$root/project" rev-parse HEAD)" = "$parent_head"
test ! -e "$cache/.git"
# Branch/tag cloning and existing-cache reuse retain their previous behavior.
git -C "$root/library" branch fixture-branch "$old"
git -C "$root/library" tag fixture-tag "$new"
for ref in fixture-branch fixture-tag; do
  mkdir -p "$root/$ref/repository"
  cp "$root/project/main.cpp" "$root/$ref/main.cpp"
  printf 'remoteRepository=""\nlocalRepository="repository"\n' > "$root/$ref/coddle.toml"
  printf '[[library]]\ntype="git"\nname="fixture"\npath="%s"\nversion="%s"\nincludes=["fixture/fixture.hpp"]\n' \
    "$root/library" "$ref" > "$root/$ref/repository/libraries.toml"
  (cd "$root/$ref" && "$binary" dep-only)
  ref_head=$(git -C "$root/library" rev-parse "$ref")
  ref_cache="$root/$ref/.coddle/libs_src/fixture"
  test "$(git -C "$ref_cache" rev-parse HEAD)" = "$ref_head"
  printf '// preserved\n' >> "$ref_cache/fixture/fixture.hpp"
  (cd "$root/$ref" && "$binary" dep-only)
  test "$(git -C "$ref_cache" rev-parse HEAD)" = "$ref_head"
  grep -q preserved "$ref_cache/fixture/fixture.hpp"
done
echo 'PASS exact-pin cache repair and preservation fixtures'
