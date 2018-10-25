.PHONY: clean verify upload

all upload:
	/home/mobaxterm/Desktop/Arduino/./arduino_debug.exe \
	--upload --verbose-upload  --verbose-build \
	'C:\Users\ysun46\My Documents\arduino\KRobot_PS2X\KRobot_PS2X.ino' \
	--port COM6
	
verify: KRobot_PS2X.ino
	/home/mobaxterm/Desktop/Arduino/./arduino_debug.exe --verify  'C:\Users\ysun46\My Documents\arduino\KRobot_PS2X\KRobot_PS2X.ino' --verbose-build


