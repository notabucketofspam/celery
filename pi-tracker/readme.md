# the part with the rapberry pi

the hardware for the camera tracking:
- Raspberry Pi 4 Model B
- Arducam UC-A73 Rev. B

some notes about things that are in this folder:
- [connection-manager.md](connection-manager.md) - how to connect this abomination to visual studio community 2026
- [build-opencv.sh](build-opencv.sh) - a script that probably builds opencv from source
- [more-setup.sh](more-setup.sh) - additional setup tasks that should probably be run on the pi

## how to make it so that there's no password for `sudo`

assuming your username is `celery`...
- `sudo visudo`
- and then put `celery ALL=(ALL:ALL) NOPASSWD: ALL` at the bottom.
 

