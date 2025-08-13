#!/bin/bash
# Script to set up a systemd service for continuous monitoring

SERVICE_NAME="environet-analyzer"
SERVICE_DESC="EnviroNet Analyzer Service"
WORKING_DIR="$(pwd)"
EXEC_START="$WORKING_DIR/.venv/bin/python $WORKING_DIR/main.py --monitor"
USER="$(whoami)"

# Create the service file
cat > /tmp/${SERVICE_NAME}.service << EOF
[Unit]
Description=${SERVICE_DESC}
After=network.target

[Service]
User=${USER}
WorkingDirectory=${WORKING_DIR}
ExecStart=${EXEC_START}
Restart=on-failure
RestartSec=5s

[Install]
WantedBy=multi-user.target
EOF

# Install the service
sudo mv /tmp/${SERVICE_NAME}.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable ${SERVICE_NAME}

echo "Service installed. Start with: sudo systemctl start ${SERVICE_NAME}"
echo "View logs with: sudo journalctl -u ${SERVICE_NAME}"