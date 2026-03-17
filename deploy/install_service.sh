#!/usr/bin/env bash
set -euo pipefail

APP_DIR=/opt/frogquant
BIN_DIR=$APP_DIR/bin
SERVICE_SRC=deploy/frogquant.service
SERVICE_DST=/etc/systemd/system/frogquant.service
LOGROTATE_SRC=deploy/frogquant.logrotate
LOGROTATE_DST=/etc/logrotate.d/frogquant
ENV_EXAMPLE=deploy/frogquant.env.example
ENV_DST=/etc/default/frogquant
HEALTHCHECK_SRC=deploy/healthcheck.sh
HEALTHCHECK_DST=$APP_DIR/deploy/healthcheck.sh
HC_SERVICE_SRC=deploy/frogquant-healthcheck.service
HC_SERVICE_DST=/etc/systemd/system/frogquant-healthcheck.service
HC_TIMER_SRC=deploy/frogquant-healthcheck.timer
HC_TIMER_DST=/etc/systemd/system/frogquant-healthcheck.timer

sudo mkdir -p $BIN_DIR
sudo mkdir -p $APP_DIR/deploy
sudo mkdir -p /var/log/frogquant
sudo cp build/frogquant_market_app $BIN_DIR/
sudo cp $SERVICE_SRC $SERVICE_DST
sudo cp $LOGROTATE_SRC $LOGROTATE_DST
sudo cp $HEALTHCHECK_SRC $HEALTHCHECK_DST
sudo chmod +x $HEALTHCHECK_DST
sudo cp $HC_SERVICE_SRC $HC_SERVICE_DST
sudo cp $HC_TIMER_SRC $HC_TIMER_DST
if [ ! -f "$ENV_DST" ]; then
  sudo cp $ENV_EXAMPLE $ENV_DST
  echo "Created $ENV_DST (please fill credentials)"
fi

sudo systemctl daemon-reload
sudo systemctl enable frogquant
sudo systemctl restart frogquant
sudo systemctl enable frogquant-healthcheck.timer
sudo systemctl restart frogquant-healthcheck.timer

echo "Done. Check status:"
echo "  systemctl status frogquant --no-pager"
echo "  systemctl status frogquant-healthcheck.timer --no-pager"
echo "  journalctl -u frogquant -f"
echo "  journalctl -u frogquant-healthcheck.service -f"
