#!/usr/bin/sh

# lets you actually use the uart and camera on the pi
sudo usermod -aG video,dialout $USER

# load the camera driver
sudo echo "dtoverlay=imx519" >> /boot/firmware/config.txt

# some changes for ssh to make it play nice with visual studio
sudo cat << EOF > /etc/ssh/sshd_config.d/00-vs-compat.conf
KexAlgorithms curve25519-sha256,curve25519-sha256@libssh.org,ecdh-sha2-nistp256,ecdh-sha2-nistp384,ecdh-sha2-nistp521,diffie-hellman-group-exchange-sha256,diffie-hellman-group14-sha256
HostKeyAlgorithms ecdsa-sha2-nistp256,ecdsa-sha2-nistp384,ecdsa-sha2-nistp521,ssh-ed25519,rsa-sha2-512,rsa-sha2-256,ssh-rsa
PubkeyAcceptedAlgorithms ecdsa-sha2-nistp256,ecdsa-sha2-nistp384,ecdsa-sha2-nistp521,ssh-ed25519,rsa-sha2-512,rsa-sha2-256,ssh-rsa
EOF

