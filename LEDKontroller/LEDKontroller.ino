/*
  Custom remote controlled led driver thingy with some basic effects.

  Uses Arduino Uno R3 to drive an RGB led / led strip connected to PWM pins 9, 10, 11.
  Program only runs the led strip while a remote control signal is sensed HIGH in Digital pin 2.
  Toggling switch state advances the LED effect program.
  (Remote control signal can be simulated by connecting a positive line through a series resistor to pin 2 via normally-closed switch.)
 
*/

// uncomment to enable debug messages
// # define DEBUG

const String VERSION = "1.1";

const int remoteIn = 2;  // Remote signal digital in pin
const int rLed = 9;  // PWM pin the red LED is attached to
const int gLed = 10;  // PWM pin the green LED is attached to
const int bLed = 11;  // PWM pin the blue LED is attached to
const int ledPins[] = {rLed, gLed, bLed};  // store leds in array for easy access

const int fastPollMillis = 100;   // Fast poll rate millis
unsigned long lastOn = 0;   // Remote signal lastOn timestamp in millis
unsigned long lastOff = 0;  // Remote signal lastOff timestamp in millis
volatile bool remoteState = false;  // remote signal toggle state


// setup the pins and interrupts.
void setup() {
  // declare remote switch input
  pinMode(remoteIn, INPUT);

  // attach interrupt to remote in
  attachInterrupt(digitalPinToInterrupt(remoteIn), updateState, CHANGE);

  // declare led output pins
  pinMode(rLed, OUTPUT);
  pinMode(gLed, OUTPUT);
  pinMode(bLed, OUTPUT);

  #ifdef DEBUG
  // setup debug logger
  Serial.begin(9600);
  Serial.println("Arduino Uno R3 LED controller V: " + VERSION);
  #endif
}

// log debug message to serial console.
void debug(String msg) {
  #ifdef DEBUG
  Serial.println(msg);
  #endif
}

// update remote switch state.
void updateState() {
  remoteState = bool(digitalRead(remoteIn));
}

/*
  returns current remote switch state and keeps track of state change timestamps.
  loops that check switch state very often should instead read variable remoteState directly.
*/
bool switchOn() {
  // check if state changed
  if (remoteState) {
    if (millisSinceLastOff() <= millisSinceLastOn()) {
      // signal used to be off, update timestamp
      lastOn = millis();
      debug("HIGH: " + String(lastOn));
    }
  }
  else if (millisSinceLastOn() < millisSinceLastOff()) {
    // signal used to be on, update timestamp
    lastOff = millis();
    debug("LOW: " + String(lastOff));
  }
  return remoteState;
}

/*
  better less-blocking delay.
  runs until period is reached or switch state turns LOW.
  use this instead of delay() to maintain responsiveness while inside a program.
*/
void belay(long period) {
  unsigned long start = millis();
  while (remoteState) {
    if ((millis() - start) >= (unsigned long)period) {
      break;
    }
  }
}

// duration in millis since last poweroff event.
unsigned long millisSinceLastOff() {
  return millis() - lastOff;
}

unsigned long millisSinceLastOn() {
  return millis() - lastOn;
}

// sets provided led pin pwm intensity 0 .. 255.
void setLed(int led, int brightness) {
  analogWrite(led, brightness);
}

// turn all led channels to zero intensity.
void ledsOff() {
  setLed(rLed, 0);
  setLed(gLed, 0);
  setLed(bLed, 0);
}


// What's the point of having a LED controller without different flashy effects?
// Here's some simple programs that can produce led effects while the control signal from remote remains high.
// "Programs" must be blocking function calls; once started they must block the loop until remote control
// signal is set to LOW again, to mark program change or just turning controller off.

// Static color: set defined RGB color in decimal values 0 .. 255
void staticColorProgram(int r = 0, int g = 0, int b = 0) {
  // set leds on and block until switch state changes
  setLed(rLed, r);
  setLed(gLed, g);
  setLed(bLed, b);
  while (switchOn()) {
    // noop
    delay(fastPollMillis);
  }
  // make sure leds are left off
  ledsOff();
}

// Fade effect: classy nice and slow sequential RGB fade
void fadeLedProgram(int fadeAmount = 5, int delayMillis = 30) {
  while (switchOn()) {
    // for each 3 led pins in turn
    for (int i=0; i<3; i++) {
      int brightness = 0;
      while (remoteState) {
        brightness = brightness + fadeAmount;
        // reverse polarity!
        if ((brightness <= 0) or (brightness >= 255)) {
          fadeAmount = -fadeAmount;
        }
        // alter brightness
        setLed(ledPins[i], brightness);
        belay(delayMillis);

        // exit loop when back at zero intensity again
        if (brightness <= 0) {
          break;
        }
      }
    }
  }
  // make sure leds are left off
  ledsOff();
}


// Alternative fade effect: classy nice and slow simultaneous RGB fade
void alt_fadeLedProgram(int fadeAmount = 5, int delayMillis = 30) {
  int brightness = 0;
  while (switchOn()) {
    brightness = brightness + fadeAmount;
    // reverse polarity!
    if ((brightness <= 0) or (brightness >= 255)) {
      fadeAmount = -fadeAmount;
    }
    // alter brightness
    setLed(rLed, brightness);
    setLed(gLed, brightness);
    setLed(bLed, brightness);
    belay(delayMillis);
  }
  // make sure leds are left off
  ledsOff();
}


// Run colors effect: sets led channels sequentially on and off at user defined epileptic pace
void runningColorsProgram(int delayMillis = 80) {
  while (switchOn()) {
    // for each 3 led pins
    for (int i=0; i<3; i++) {
      setLed(ledPins[i], 255);
      belay(delayMillis);
      setLed(ledPins[i], 0);
    }
  }
  // make sure leds are left off
  ledsOff();
}


// Blink effect: sets all led channels simultaneously on and off at user defined epileptic pace
void blinkLedProgram(int delayMillis = 250) {
  while (switchOn()) {
    // set maximum brightness
    setLed(rLed, 255);
    setLed(gLed, 255);
    setLed(bLed, 255);
    belay(delayMillis);

    ledsOff();
    belay(delayMillis/2);
  }
  // make sure leds are left off
  ledsOff();
}


// Color blaster: generates a completely new random color every defined interval
void colorBlasterProgram(int delayMillis = 500) {
  // Set all led channels to random intensity, creating a random vomit of color output!
  while (switchOn()) {
    setLed(rLed, random(1, 256));
    setLed(gLed, random(1, 256));
    setLed(bLed, random(1, 256));
    belay(delayMillis);
  }
  // make sure leds are left off
  ledsOff();
}


// Alternative color blaster: generates a more gradual new random color every defined interval
void alt_colorBlasterProgram(int delayMillis = 250) {
  // Set random color to start with
  setLed(rLed, random(1, 256));
  setLed(gLed, random(1, 256));
  setLed(bLed, random(1, 256));

  // Start setting random channels to random intensity, creating a random vomit of color output!
  while (switchOn()) {
    setLed(ledPins[random(3)], random(1, 256));
    belay(delayMillis);
  }
  // make sure leds are left off
  ledsOff();
}


// "PROGRAM MANAGER" loop to keep track of remote signal state
// and advances through different programs accordingly
void loop() {
  int program = 0;  // program counter
  int fadeAmount = 0;
  int delayMillis = 0;
  while (true) {
    // call switchOn() to update timestamps and get remote signal state
    if (switchOn()) {
      switch (program) {

        // run program 1: Static color.
        case 0:
          debug("PROG 1: Static Color");
          // r, g, b values 0 .. 255
          staticColorProgram(25, 255, 25);
          break;

        // run program 2: Fancy led fader effect.
        case 1:
          debug("PROG 2: Fancy Fade");
          fadeAmount=5;
          delayMillis=30;
          fadeLedProgram(fadeAmount, delayMillis);
          break;

        // run program 3: Alternative Fancy led fader effect.
        case 2:
          debug("PROG 3: Alt Fancy Fade");
          fadeAmount=5;
          delayMillis=30;
          alt_fadeLedProgram(fadeAmount, delayMillis);
          break;

        // run program 4: Running colors.
        case 3:
          debug("PROG 4: Running Colors");
          delayMillis=80;
          runningColorsProgram(delayMillis);
          break;

        // run program 5: Epileptic disco mode for maniacs!
        case 4:
          debug("PROG 5: Maniac Disco");
          delayMillis=350;
          blinkLedProgram(delayMillis);
          break;

        // run program 6: Random changing color.
        case 5:
          debug("PROG 6: Random Color");
          delayMillis=500;
          colorBlasterProgram(delayMillis);
          break;

        // run program 7: Alternative more gradual changing color.
        case 6:
          debug("PROG 7: Alt Random Color");
          delayMillis=250;
          alt_colorBlasterProgram(delayMillis);
          break;

        // run program 8: Your own Hella Cool led program goes here
        // case 7:
        //   debug("PROG 8: Your very own Hella Cool custom program!")

        // program counter overrun.
        default:
          program = 0;
          // continue main loop
          continue;
          break;
      }
      // advance program counter
      program += 1;
    }

    // enable hibernation mode ~10000 millis after remote signal is turned off
    else if (millisSinceLastOff() > (unsigned long)(10000)) {
      debug("HIBERNATE: " + String(millis()));
      // reset program counter to simulate "off"
      program = 0;
      // observe remoteState directly to reduce lifting while waiting
      while (!remoteState) {
        // noop
        delay(fastPollMillis);
      }
    }
  }
}
