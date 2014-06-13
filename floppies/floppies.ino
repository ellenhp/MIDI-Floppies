#include "hardware_defs.h"
#include "midi.h"

unsigned int micros_per_tick=1; //TODO MAKE MEANINGFUL

byte current_pos[NUM_FLOPPIES];
byte current_dir[NUM_FLOPPIES];
byte current_note[NUM_FLOPPIES];
unsigned int period_ticks[NUM_FLOPPIES];
unsigned int ticks_since_step[NUM_FLOPPIES];

void setupPins();
void tick();
void playNote(byte note, byte velocity);
void stopNote(byte note);
void stepFloppy(byte floppy, byte dir);
void resetAll();
unsigned int period_from_note_number(byte note);

void setup() {
  resetAll(); //move the heads to one extreme at start
  SERIAL_BEGIN(MIDI_BAUD);
}

void loop() {
  byte incomingByte;
  byte note;
  byte velocity;
  if (SERIAL_AVAILABLE()>0) {
    // read the incoming byte:
    incomingByte = SERIAL_READ();

    // wait for as status-byte, channel 1, note on or off
    if (incomingByte == 144) { // note on message starting starting
      note=SERIAL_READ();
      velocity=SERIAL_READ();
      playNote(note, velocity);
    }
    else if (incomingByte == 128) { // note off message starting
      note=SERIAL_READ();
      stopNote(note);
    }
    else {
      //do nothing
    }
  }
}

void setupPins() {
  for (byte i=0; i<NUM_FLOPPIES; i++) {
    pinMode(STEP_PIN(i), OUTPUT);
    pinMode(DIR_PIN(i), OUTPUT);
    pinMode(GND_PIN(i), OUTPUT);
    digitalWrite(GND_PIN(i), LOW); //set the ground pin 
  }
}

void tick() {
  for (byte i=0; i<NUM_FLOPPIES; i++) {
    if (period_ticks[i]!=0) {
      ticks_since_step[i]++;
      //if this floppy is due to be stepped again
      if (ticks_since_step[i] > period_ticks[i++]) {
        //switch directions if needed
        if ((current_pos[i]==0 && current_dir[i]==0) || (current_pos[i]==(NUM_TRACKS-1) && current_dir[i]==1)) {
          current_dir[i]=!current_dir[i];
        }
        stepFloppy(i, current_dir[i]);
        //start counting from zero again
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
  unsigned int desiredIndexAvailableTracks=0; // the number of tracks at this index
  byte desiredDir;

  unsigned int period=PERIOD_BY_NOTE(note)/micros_per_tick;
  if (period==0) {
    return;
  }

  for (floppy=0; floppy<NUM_FLOPPIES; floppy++) {
    //if this floppy is not in use
    if (current_note[floppy]==0) {
      byte tmpDir;
      byte availableTracks;
      //figure out in which direction it has more tracks available (avoids unnecessary direction-switching)
      if (current_pos[floppy]<(NUM_TRACKS/2)) {
        availableTracks=NUM_TRACKS-1-current_pos[floppy];
        tmpDir=0;
      } else {
        availableTracks=current_pos[floppy]-1;
        tmpDir=1;
      }
      //if it's better than the previous option, replace those values
      if (availableTracks>desiredIndexAvailableTracks) {
        desiredIndex=floppy;
        desiredIndexAvailableTracks=availableTracks;
        desiredDir=tmpDir;
      }
    }
  }
  //if we found something
  if (desiredIndex!=-1) {
    //set the values indicating what the floppy is doing
    period_ticks[desiredIndex]=period;
    current_note[desiredIndex]=note;
    current_dir[desiredIndex]=desiredDir;
    
    //step it now to get the note started ASAP
    stepFloppy(desiredIndex, desiredDir);
    ticks_since_step[desiredIndex]=0;
  }
}

void stepFloppy(byte floppy, byte dir) {
  digitalWrite(DIR_PIN(floppy), dir ? HIGH : LOW);
  digitalWrite(STEP_PIN(floppy), HIGH);
  digitalWrite(STEP_PIN(floppy), LOW);
  
  //don't let the current_pos move beyond the number of tracks
  if (current_pos[floppy]<(NUM_TRACKS-1) && dir) {
    current_pos[floppy]++;
  } else if (current_pos[floppy]>0 && !dir) {
    current_pos[floppy]--;
  }
}

void resetAll() {
  for (int i=0; i<NUM_TRACKS; i++) {
    for (byte floppy=0; floppy<NUM_FLOPPIES; floppy++) {
      stepFloppy(floppy, 0);
    }
  }
}


