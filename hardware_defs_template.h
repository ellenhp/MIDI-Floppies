
//these should be more or less constant
#define NUM_TRACKS 80
#define MIDI_BAUD 31250

//these will surely change from one instrument to the next
#define NUM_FLOPPIES 10
#define START_PIN 22

//these will change depending on how the floppies are wired up
#define STEP_PIN(FLOPPY_NUM) (START_PIN+FLOPPY_NUM*3)
#define DIR_PIN(FLOPPY_NUM) (START_PIN+1+FLOPPY_NUM*3)
#define GND_PIN(FLOPPY_NUM) (START_PIN+2+FLOPPY_NUM*3)

//change these if your MIDI-in is connected to a different UART on an Arduino Mega
#define SERIAL_READ Serial.read
#define SERIAL_AVAILABLE Serial.available
#define SERIAL_BEGIN Serial.begin
