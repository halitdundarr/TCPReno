#!/bin/sh
set -eu

ROOT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
TMP_DIR="$(mktemp -d "${TMPDIR:-/tmp}/tcpreno-tests.XXXXXX")"
BIN="./tcp_sim"
NODE_BIN="./node"

cleanup() {
    rm -rf "$TMP_DIR"
}
trap cleanup EXIT
cd "$ROOT_DIR"

run_and_diff() {
    name="$1"
    expected="$2"
    shift 2
    actual="$TMP_DIR/$name.txt"
    "$BIN" "$@" >"$actual"
    if ! diff -u "$ROOT_DIR/$expected" "$actual" >/dev/null; then
        echo "FAIL: $name output differs from $expected"
        diff -u "$ROOT_DIR/$expected" "$actual" || true
        exit 1
    fi
    echo "PASS: $name"
}

run_expected_failure() {
    name="$1"
    expected="$2"
    shift 2
    actual="$TMP_DIR/$name.txt"
    if "$BIN" "$@" >"$actual" 2>&1; then
        echo "FAIL: $name was expected to fail"
        exit 1
    fi
    if ! diff -u "$ROOT_DIR/$expected" "$actual" >/dev/null; then
        echo "FAIL: $name stderr/stdout differs from $expected"
        diff -u "$ROOT_DIR/$expected" "$actual" || true
        exit 1
    fi
    echo "PASS: $name"
}

run_node_smoke() {
    name="$1"
    expected="$2"
    shift 2
    actual="$TMP_DIR/$name.txt"
    if ! "$NODE_BIN" --dry-run "$@" >"$actual" 2>&1; then
        echo "FAIL: $name node wrapper exited unexpectedly"
        cat "$actual" || true
        exit 1
    fi
    if ! diff -u "$ROOT_DIR/$expected" "$actual" >/dev/null; then
        echo "FAIL: $name output differs from $expected"
        diff -u "$ROOT_DIR/$expected" "$actual" || true
        exit 1
    fi
    echo "PASS: $name"
}

echo "Running TCPReno regression checks..."
run_and_diff "reno_timeout" "results/reno_timeout.txt" --algo reno --scenario "scenarios/timeout_case.txt"
run_and_diff "reno_triple_dup" "results/reno_triple_dup.txt" --algo reno --scenario "scenarios/triple_dup_case.txt"
run_and_diff "tahoe_timeout" "results/tahoe_timeout.txt" --algo tahoe --scenario "scenarios/timeout_case.txt"
run_and_diff "tahoe_triple_dup" "results/tahoe_triple_dup.txt" --algo tahoe --scenario "scenarios/triple_dup_case.txt"
run_expected_failure "invalid_algo" "results/negative_invalid_algo.txt" --algo bogus --scenario "scenarios/timeout_case.txt"
run_expected_failure "invalid_event" "results/negative_invalid_event.txt" --algo reno --scenario "scenarios/invalid_event_case.txt"
run_node_smoke "node_wrapper_start" "results/node_wrapper_start.txt" "A.conf"
echo "All regression checks passed."
