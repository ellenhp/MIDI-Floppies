#include "hardware_defs.h"
#include "midi.h"

#include "TimerOne.h"

unsigned int micros_per_tick=40;

byte current_pos[NUM_FLOPPIES];
byte current_dir[NUM_FLOPPIES];
unsigned int period_ticks[NUM_FLOPPIES];
unsigned int ticks_since_step[NUM_FLOPPIES];

byte oscillate=1;
byte note=0;

void setupPins();
void tick();
void stepFloppy(byte floppy, byte dir);
void resetAll();

void setup() {
  setupPins();
  resetAll(); // move the heads to one extreme at start
  // if desired, testAll() should be called here
  Timer1.initialize(micros_per_tick); // Set up a timer at the defined resolution
  Timer1.attachInterrupt(tick); // Attach the tick function
  Serial.begin(9600);
}

void loop() {
  Serial.print("Drive number?\n");
  while (!Serial.available());
  long driveNum=Serial.parseInt()-1;
  while (Serial.available()) {
    Serial.read();
  }
  if (driveNum>=0 && driveNum<NUM_FLOPPIES) {
    while (1) {
      if (note!=0) {
        Serial.print(note);
        Serial.print(" - ");
      }
      Serial.print("Note number? (0 to toggle note mode, space to step by one)\n");
      while (!Serial.available());
      if (Serial.peek()==' ' && note!=0) {
        while (Serial.available()) {
          Serial.read();
        }
        note++;
        period_ticks[driveNum]=PERIOD_BY_NOTE(note)/micros_per_tick;
      }
      else {
        long noteNum=Serial.parseInt();
        while (Serial.available()) {
          Serial.read();
        }
        if (noteNum==0) {
          oscillate=!oscillate;
        }
        else {
          note=noteNum;
          period_ticks[driveNum]=PERIOD_BY_NOTE(note)/micros_per_tick;
        }
      }
    }
  }
}

void setupPins() {
  for (byte i=0; i<NUM_FLOPPIES; i++) {
    pinMode(STEP_PIN(i), OUTPUT);
    pinMode(DIR_PIN(i), OUTPUT);
    pinMode(GND_PIN(i), OUTPUT);
    digitalWrite(GND_PIN(i), LOW); // set the ground pin 
  }
}

void tick() {
  for (byte i=0; i<NUM_FLOPPIES; i++) {
    if (period_ticks[i]!=0) {
      ticks_since_step[i]++;
      // if this floppy is due to be stepped again
      if (ticks_since_step[i] > period_ticks[i]) {
        // switch directions if needed
        if (oscillate || (current_pos[i]==0 && current_dir[i]==0) || (current_pos[i]==(NUM_TRACKS-1) && current_dir[i]==1)) {
          current_dir[i]=!current_dir[i];
        }
        stepFloppy(i, current_dir[i]);
        // start counting from zero again
        ticks_since_step[i]=0;
      }
    }
  }
}

void stepFloppy(byte floppy, byte dir) {
  digitalWrite(DIR_PIN(floppy), dir ? HIGH : LOW);
  digitalWrite(STEP_PIN(floppy), HIGH);
  digitalWrite(STEP_PIN(floppy), LOW);

  // don't let the current_pos move beyond the number of tracks
  if (current_pos[floppy]<(NUM_TRACKS-1) && dir) {
    current_pos[floppy]++;
  } 
  else if (current_pos[floppy]>0 && !dir) {
    current_pos[floppy]--;
  }
}

void resetAll() {
  for (byte i=0; i<NUM_TRACKS; i++) {
    for (byte floppy=0; floppy<NUM_FLOPPIES; floppy++) {
      stepFloppy(floppy, 0);
    }
    delay(5);
  }
}


