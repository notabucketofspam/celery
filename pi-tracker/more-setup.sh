#!/usr/bin/sh

# lets you actually use the uart and camera on the pi
sudo usermod -aG video,dialout $USER

# load some other drivers
sudo cat << EOF >> /boot/firmware/config.txt
dtoverlay=imx519
dtparam=spi=on
dtoverlay=piscreen,speed=16000000,rotate=90
dtoverlay=uart5
EOF

# some changes for ssh to make it play nice with visual studio
sudo cat << EOF > /etc/ssh/sshd_config.d/00-vs-compat.conf
KexAlgorithms curve25519-sha256,curve25519-sha256@libssh.org,ecdh-sha2-nistp256,ecdh-sha2-nistp384,ecdh-sha2-nistp521,diffie-hellman-group-exchange-sha256,diffie-hellman-group14-sha256
HostKeyAlgorithms ecdsa-sha2-nistp256,ecdsa-sha2-nistp384,ecdsa-sha2-nistp521,ssh-ed25519,rsa-sha2-512,rsa-sha2-256,ssh-rsa
PubkeyAcceptedAlgorithms ecdsa-sha2-nistp256,ecdsa-sha2-nistp384,ecdsa-sha2-nistp521,ssh-ed25519,rsa-sha2-512,rsa-sha2-256,ssh-rsa
EOF

# we love tmux
sudo apt install -y tmux
cat << EOF > ~/.tmux.conf
set -g mouse on
EOF

# remove the login prompt
sudo systemctl disable --now getty@tty1.service

# dont fall asleep
sudo sed -i '$ s/$/ consoleblank=0 vt.global_cursor_default=0 logo.nologo/' /boot/firmware/cmdline.txt

# start the app on startup
sudo cat << EOF > /etc/systemd/system/dronetracker.service
[Unit]
Description=DroneTracker OpenCV Service
After=multi-user.target

[Service]
Type=simple
User=celery
WorkingDirectory=/home/celery/.vs/celery/out/build/pi-linux-debug/pi-tracker/
ExecStart=/home/celery/.vs/celery/out/build/pi-linux-debug/pi-tracker/DroneTracker
Restart=always
RestartSec=5
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
EOF

sudo systemctl daemon-reload
sudo systemctl enable dronetracker.service
sudo systemctl start dronetracker.service

# kms
sudo init 6
