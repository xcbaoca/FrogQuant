#!/usr/bin/env bash
set -euo pipefail

URL=${FQ_HEALTH_URL:-http://127.0.0.1:8080/health}
LOG=${FQ_HEALTHCHECK_LOG:-/var/log/frogquant/healthcheck.log}
ALERT=${FQ_HEALTHCHECK_ALERT_CMD:-}

mkdir -p "$(dirname "$LOG")"

TS=$(date '+%Y-%m-%d %H:%M:%S')
TMP_BODY=$(mktemp)
trap 'rm -f "$TMP_BODY"' EXIT

HTTP_CODE=$(curl -sS -m 3 -o "$TMP_BODY" -w '%{http_code}' "$URL" || echo 000)
BODY=$(cat "$TMP_BODY" 2>/dev/null || echo '{}')

if [[ "$HTTP_CODE" != "200" ]]; then
  echo "[$TS] UNHEALTHY code=$HTTP_CODE body=$BODY" >> "$LOG"
  if [[ -n "$ALERT" ]]; then
    bash -lc "$ALERT 'FrogQuant healthcheck failed: code=$HTTP_CODE body=$BODY'" || true
  fi
  exit 1
else
  echo "[$TS] OK code=$HTTP_CODE body=$BODY" >> "$LOG"
fi
