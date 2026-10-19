# the part with the rapberry pi

the hardware for the camera tracking:
- Raspberry Pi 4 Model B
- Arducam UC-A73 Rev. B

some notes about things that are in this folder:
- [connection-manager.md](connection-manager.md) - how to connect this abomination to visual studio community 2026
- [build-opencv.sh](build-opencv.sh) - a script that probably builds opencv from source
	- I highly recommend that you `sudo apt install -y tmux` and then `tmux` beforehand, because the opencv build process can take a couple of hours.
- [more-setup.sh](more-setup.sh) - additional setup tasks that should probably be run on the pi

## how to actually build the raspberry pi part

- at the top, there's a dropdown. it probably says **"Local Machine"** at the start.
- change it to **"celery.local"** (or whatever your pi is named idk)
- once that loads, change the dropdown right next to it to **"Build Pi DroneTracker"**
- hit `ctrl + shift + b` to build it
- the binary ends up in `/home/celery/.vs/celery/out/build/pi-linux-debug/pi-tracker/DroneTracker`	- so maybe just make a symlink to it
		- `sudo ln -s /home/celery/.vs/celery/out/build/pi-linux-debug/pi-tracker/DroneTracker /usr/local/bin/DroneTracker`

**IMPORTANT**: you need to go through all of the [Connection Manager Trials of Hercules](./connection-manager.md)
before you can do the building for the raspberry pi, since this actually does all of the compilation on
the pi herself.

## how to make it so that there's no password for `sudo`

assuming your username is `celery`...
- `sudo visudo`
- and then put `celery ALL=(ALL:ALL) NOPASSWD: ALL` at the bottom.
- and also save n exit, too.
- and log out and log back in (i think).
