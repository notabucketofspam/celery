# celery

##### Episode V: The Sloppotron Strikes Back

what is celery?

---

how to open this with visual studio community 2026:
- file > open > CMake project
- navigate to the `CMakeLists.txt` file which is [right here](CMakeLists.txt)
- hit the open button

if you want to build the raspberry pi part:
- at the top, there's a dropdown. it probably says "Local Machine" at the start.
- change it to "celery.local" (or whatever your pi is named idk)
- once that loads, change the dropdown right next to it to "Build Pi DroneTracker"
- hit `ctrl + shift + b` to build it

**IMPORTANT**: you need to go through all of the Configuration Manager Trials of Hercules
before you can do the building for the pi, since it actually does all of the compilation on
the pi itself.

if you want to build the arduino mega part:
- basically the same steps, but it's the "Local Machine" option instead
- and then in the other dropdown, it's the "Build AVR Firmware (.hex)"
- `ctrl + shift + b` again
- after that, you can use the "Flash AVR via avrdude (upload)" option and 
`ctrl + shift + b` again to actually flash it.

**IMPORTANT**: make sure to change the com port in the [`ramps-firmware/CMakeLists.txt`](./ramps-firmware/CMakeLists.txt) so that it's actually the com port that you wanna use.

what's the deal with this documentation?
- sometimes i forget things
- so i gotta write it down
- in case i need to refer back to it later

---

please look both ways before you cross the street.
