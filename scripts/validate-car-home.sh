#!/usr/bin/env bash
set -euo pipefail

# Validate AAOS post-boot launcher state for a target user.
# Fails when the target user is stuck in BOOTING or HOME resolves to fallback.

TARGET_USER="${1:-}"
EXPECTED_HOME="${2:-com.android.car.carlauncher}"
FALLBACK_HOME="${3:-com.android.car.settings/.FallbackHome}"

if ! command -v adb >/dev/null 2>&1; then
  echo "ERROR: adb not found in PATH."
  exit 2
fi

if [[ -z "${TARGET_USER}" ]]; then
  TARGET_USER="$(adb shell am get-current-user | tr -d '\r' | tr -d '\n' || true)"
fi

if [[ -z "${TARGET_USER}" ]]; then
  echo "ERROR: Could not determine target user."
  echo "Usage: $0 <user-id> [expected-home-package] [fallback-home-component]"
  exit 2
fi

echo "== AAOS Home Validation =="
echo "Target user: ${TARGET_USER}"
echo "Expected home package: ${EXPECTED_HOME}"
echo "Forbidden fallback: ${FALLBACK_HOME}"
echo

CURRENT_USER="$(adb shell am get-current-user | tr -d '\r' | tr -d '\n')"
echo "Current foreground user: ${CURRENT_USER}"

STATE_LINE="$(adb shell dumpsys user | tr -d '\r' | rg 'Started users state' || true)"
echo "Started users state: ${STATE_LINE:-<missing>}"
if [[ -z "${STATE_LINE}" ]]; then
  echo "ERROR: Could not read Started users state."
  exit 3
fi

if echo "${STATE_LINE}" | rg -q "${TARGET_USER}=BOOTING"; then
  echo "ERROR: User ${TARGET_USER} is still BOOTING."
  echo "Hint: wait/unlock the profile, or reboot and re-check."
  exit 4
fi

RESOLVE_OUTPUT="$(adb shell cmd package resolve-activity --brief --user "${TARGET_USER}" -a android.intent.action.MAIN -c android.intent.category.HOME | tr -d '\r' || true)"
echo
echo "HOME resolve output:"
echo "${RESOLVE_OUTPUT:-<empty>}"

if [[ -z "${RESOLVE_OUTPUT}" ]] || echo "${RESOLVE_OUTPUT}" | rg -qi '^No activity found'; then
  echo "ERROR: No HOME activity resolved for user ${TARGET_USER}."
  exit 5
fi

RESOLVED_COMPONENT="$(echo "${RESOLVE_OUTPUT}" | rg '^[^=].*/.+' | tail -n 1 || true)"
if [[ -z "${RESOLVED_COMPONENT}" ]]; then
  echo "ERROR: Could not parse resolved HOME component."
  exit 6
fi

if [[ "${RESOLVED_COMPONENT}" == "${FALLBACK_HOME}" ]]; then
  echo "ERROR: HOME resolves to fallback (${FALLBACK_HOME}), not CarLauncher."
  exit 7
fi

if ! echo "${RESOLVED_COMPONENT}" | rg -q "^${EXPECTED_HOME}/"; then
  echo "ERROR: HOME resolves to unexpected package: ${RESOLVED_COMPONENT}"
  exit 8
fi

TOP_LINE="$(adb shell dumpsys activity activities | tr -d '\r' | rg -i 'topResumedActivity=' | head -n 1 || true)"
echo
echo "Top resumed activity: ${TOP_LINE:-<missing>}"
if [[ -n "${TOP_LINE}" ]] && echo "${TOP_LINE}" | rg -q "${FALLBACK_HOME}"; then
  echo "ERROR: Fallback home is currently top resumed activity."
  exit 9
fi

echo
echo "PASS: user state and HOME resolver look healthy."
