.PHONY: clean verify upload

all:
	/home/mobaxterm/Desktop/Arduino/./arduino_debug.exe \
	--upload --verbose-upload  --verbose-build --preserve-temp-files \
	'C:\Users\ysun46\My Documents\arduino\KRobot_PS2X\KRobot_PS2X.ino' \
	--port COM8
	
verify: KRobot_PS2X.ino
	/home/mobaxterm/Desktop/Arduino/./arduino_debug.exe --verify --preserve-temp-files 'C:\Users\ysun46\My Documents\arduino\KRobot_PS2X\KRobot_PS2X.ino' --verbose-build

upload:
	/mnt/c/Users/ysun46/Desktop/Arduino/hardware/tools/avr/bin/avrdude.exe \
	-C./avrdude.conf -v -patmega328p -carduino -PCOM8 -b115200 -D \
	-Uflash:w:./KRobot_PS2X.cpp.hex:i
