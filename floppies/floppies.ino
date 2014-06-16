#include "hardware_defs.h"
#include "midi.h"

#include "TimerOne.h"

byte max_notes_by_floppy[]=FLOPPY_MAX_NOTES;

unsigned int micros_per_tick=40;

byte current_pos[NUM_FLOPPIES];
byte current_dir[NUM_FLOPPIES];
byte current_note[NUM_FLOPPIES];
unsigned int period_ticks[NUM_FLOPPIES];
unsigned int ticks_since_step[NUM_FLOPPIES];
byte last_cmd=0;

void setupPins();
void tick();
void playNote(byte note, byte velocity);
void stopNote(byte note);
void stepFloppy(byte floppy, byte dir);
void resetAll();
void pulseAll();
void testAll();

void setup() {
  setupPins();
  // if desired, resetAll() and then testAll() should be called here
  Timer1.initialize(micros_per_tick); // Set up a timer at the defined resolution
  Timer1.attachInterrupt(tick); // Attach the tick function
  SERIAL_BEGIN(MIDI_BAUD);
}

void loop() {
  byte incomingByte;
  byte note;
  byte velocity;

  if (SERIAL_AVAILABLE()>0) {
    // read the incoming byte:
    incomingByte = SERIAL_READ();
    if (incomingByte == 144) { // note on message starting
      while (!SERIAL_AVAILABLE());
      note=SERIAL_READ();
      while (!SERIAL_AVAILABLE());
      velocity=SERIAL_READ();
      playNote(note, velocity);
      last_cmd=144;
    }
    else if (incomingByte == 128) { // note off message starting
      while (!SERIAL_AVAILABLE());
      note=SERIAL_READ();
      while (!SERIAL_AVAILABLE());
      SERIAL_READ();
      stopNote(note);
      last_cmd=128;
    } 
    else if (incomingByte < 128 && last_cmd == 144) { // data byte, assume it's a continuation of the last command
      note=incomingByte;
      while (!SERIAL_AVAILABLE());
      velocity=SERIAL_READ();
      playNote(note, velocity);
    } 
    else if (incomingByte < 128 && last_cmd == 128) { // data byte, assume it's a continuation of the last command
      while (!SERIAL_AVAILABLE());
      note=incomingByte;
      while (!SERIAL_AVAILABLE());
      SERIAL_READ();
      stopNote(note);
    }
    else {
      // no other commands implemented
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
        // switch directions and step
        current_dir[i]=!current_dir[i];
        stepFloppy(i, current_dir[i]);
        // start counting from zero again
        ticks_since_step[i]=0;
      }
    }
  }
}

void stopNote(byte note) {
  byte i;
  for (i=0; i<NUM_FLOPPIES; i++) {
    if (current_note[i]==note) {
      current_note[i]=0;
      period_ticks[i]=0;
    }
  }
}

void playNote(byte note, byte velocity) {
  if (velocity==0) {
    stopNote(note);
  }
  byte floppy;
  signed int desiredIndex=-1; // the index of the floppy that we want to use

  unsigned int period=PERIOD_BY_NOTE(note)/micros_per_tick;
  if (period==0) {
    return;
  }

  for (floppy=0; floppy<NUM_FLOPPIES; floppy++) {
    // if this floppy is not in use
    if (current_note[floppy]==0) {
      if (note<=MAX_NOTE_BY_FLOPPY(floppy)) {
        //use this floppy
        desiredIndex=floppy;
        break;
      }
    }
  }
  // if we found something
  if (desiredIndex!=-1) {
    // set the values indicating what the floppy is doing
    period_ticks[desiredIndex]=period;
    current_note[desiredIndex]=note;

    // step it now to get the note started ASAP
    stepFloppy(desiredIndex, current_dir[desiredIndex]);
    ticks_since_step[desiredIndex]=0;
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

void pulseAll() {
  for (byte floppy=0; floppy<NUM_FLOPPIES; floppy++) {
    stepFloppy(floppy, 1);
  }
  delay(5);
  for (byte floppy=0; floppy<NUM_FLOPPIES; floppy++) {
    stepFloppy(floppy, 0);
  }
}

void testAll() {
  pulseAll();
  for (byte floppy=0; floppy<NUM_FLOPPIES; floppy++) {
    for (byte track=0; track<NUM_TRACKS; track++) {
      stepFloppy(floppy, 1);
      delay(5);
    }
    pulseAll();
    for (byte track=0; track<NUM_TRACKS; track++) {
      stepFloppy(floppy, 0);
      delay(5);
    }
    pulseAll();
  }
}


