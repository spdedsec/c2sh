#!/bin/sh
set -eu
BIN=$1
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
out=$($BIN -c 'printf "a\\nb\\n" | grep b')
test "$out" = b
$BIN -c 'printf hello > '"$TMP"'/x'
test "$(cat "$TMP/x")" = hello
$BIN -c 'false && printf bad > '"$TMP"'/bad; true || printf bad > '"$TMP"'/bad; printf ok > '"$TMP"'/ok'
test -f "$TMP/ok"
test ! -f "$TMP/bad"
$BIN -c 'sleep 0.05 &' >/dev/null
$BIN -c 'echo "$HOME"' >/dev/null
echo "integration tests: ok"
