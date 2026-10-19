# the part with the arduino mega

hardware for the stepper motors:
- any Arduino Mega 2560 knockoff
- RAMPS 1.6 board, or something akin to that
- two (2) A4988 stepper drivers (or equivalent)
- two (2) NEMA 17 stepper motors (200 steps/rev, 1.8 deg/step, any voltage / current / etc.)

computer software that you need to download and then put in your system `path`:
- [the avr-gcc toolchain](https://www.microchip.com/en-us/tools-resources/develop/microchip-studio/gcc-compilers)
  - on the path, it should look more-or-less like `C:\opt\atmel\avr8-gnu-toolchain\bin`
  - at least, that's what the cmake project right here assumes
- [avrdude](https://github.com/avrdudes/avrdude)
  - on the path, it's `C:\opt\atmel`
- you can get to the "Edit Environment Variables" window super quickly with `C:\Windows\System32\rundll32.exe sysdm.cpl,EditEnvironmentVariables`
  - and you can put this into a shortcut, too.

other source files that you need to download and then put in the `lib` folder:
- [the `lib` folder is right here btw](./lib)
- [AccelStepper](https://www.airspayce.com/mikem/arduino/AccelStepper/)
  - the folder name can stay as `AccelStepper`. just make sure you extract it, pal.
- [ArduinoCore-avr](https://github.com/arduino/arduinocore-avr)
  - gotta extract and then rename the folder to `arduino_core`
  
