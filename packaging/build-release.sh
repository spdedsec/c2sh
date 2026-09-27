#!/bin/sh
set -eu
VERSION=${1:-0.1.0}
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$ROOT"
make clean all test
rm -rf dist
mkdir -p dist
NAME="c2sh-$VERSION"
TMP="dist/$NAME"
mkdir -p "$TMP"
cp -a README.md LICENSE Makefile CHANGELOG.md include src tests docs packaging .github "$TMP/"
tar -C dist -czf "dist/$NAME.tar.gz" "$NAME"
rm -rf "$TMP"
sha256sum "dist/$NAME.tar.gz" > "dist/$NAME.tar.gz.sha256"
printf '%s\n' "Created dist/$NAME.tar.gz"
