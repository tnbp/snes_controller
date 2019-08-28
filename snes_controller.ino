/** snes_controller.ino
 *  A sketch for a 2-port SNES-to-USB converter
 *  powered by Arduino Leonardo (or compatibles)
 *  
 *  heavily based on "Arduino Micro SNES Controller.ino" by Anthony Burkholder (burks10)
 *  https://github.com/burks10/Arduino-SNES-Controller
 *  
 *  Vital information about SNES hardware was taken from here:
 *  https://gamefaqs.gamespot.com/snes/916396-super-nintendo/faqs/5395
 *  
 *  To get both gamepads to work on Linux, please follow the following instructions:
 *  http://mheironimus.blogspot.com/2015/09/linux-support-for-arduino-leonardo.html
 **/

#include <Joystick.h>

/** PINS **/
int DATA_CLOCK_1    = 2;
int DATA_LATCH_1    = 3;
int DATA_SERIAL_1   = 4;

int DATA_CLOCK_2    = 6;
int DATA_LATCH_2    = 7;
int DATA_SERIAL_2   = 9;

/** Data store for current state of buttons **/
int buttons_1[16];
int buttons_2[16];

/** "Skip" flag for controllers (if disconnected) **/
bool skip_j1;
bool skip_j2;

/** Remap buttons so the order is more logical (B Y A X L R Select Start)
 *  Original order is: B Y Select Start Up Down Left Right A X L R n/a n/a n/a n/a
 */
int button_map[16] = {0, 1, 6, 7, 8, 9, 10, 11, 2, 3, 4, 5, 12, 13, 14, 15};

/** Init joysticks **/
#define JOYSTICK_COUNT 2

Joystick_ Joystick[JOYSTICK_COUNT] = {
  Joystick_(0x03, JOYSTICK_TYPE_GAMEPAD, 8, 0, true, true, false, false, false, false, false, false, false, false, false),
  Joystick_(0x04, JOYSTICK_TYPE_GAMEPAD, 8, 0, true, true, false, false, false, false, false, false, false, false, false)
};

void setup() {
  Joystick[0].begin();
  Joystick[0].setXAxisRange(-127, 127);
  Joystick[0].setYAxisRange(-127, 127);
  Joystick[1].begin();
  Joystick[1].setXAxisRange(-127, 127);
  Joystick[1].setYAxisRange(-127, 127);
  setupPins();
}

void loop() {
  RXTXControllerData();
}

void setupPins() {
  /** Set DATA_CLOCK normally HIGH **/
  pinMode(DATA_CLOCK_1, OUTPUT);
  digitalWrite(DATA_CLOCK_1, HIGH);
  
  /** Set DATA_LATCH normally LOW **/
  pinMode(DATA_LATCH_1, OUTPUT);
  digitalWrite(DATA_LATCH_1, LOW);

  /** Set DATA_SERIAL normally HIGH **/
  pinMode(DATA_SERIAL_1, OUTPUT);
  digitalWrite(DATA_SERIAL_1, HIGH);
  pinMode(DATA_SERIAL_1, INPUT);  
  
  /** ------------------------
  Repeat for second controller **/
  pinMode(DATA_CLOCK_2, OUTPUT);
  digitalWrite(DATA_CLOCK_2, HIGH);
  
  pinMode(DATA_LATCH_2, OUTPUT);
  digitalWrite(DATA_LATCH_2, LOW);

  pinMode(DATA_SERIAL_2, OUTPUT);
  digitalWrite(DATA_SERIAL_2, HIGH);
  pinMode(DATA_SERIAL_2, INPUT);  
}

void RXTXControllerData() {
  skip_j1 = skip_j2 = false;
  /** Latch for 12us **/
  digitalWrite(DATA_LATCH_1, HIGH);
  digitalWrite(DATA_LATCH_2, HIGH);
  delayMicroseconds(12);
  digitalWrite(DATA_LATCH_1, LOW);
  digitalWrite(DATA_LATCH_2, LOW);
  delayMicroseconds(6);

  /** Read data bit by bit from SR **/
  for (int i = 0; i < 16; i++) {
    digitalWrite(DATA_CLOCK_1, LOW);
    digitalWrite(DATA_CLOCK_2, LOW);
    delayMicroseconds(6);
    if (i <= 15) {
      buttons_1[i] = digitalRead(DATA_SERIAL_1);
      buttons_2[i] = digitalRead(DATA_SERIAL_2);
    }
    digitalWrite(DATA_CLOCK_1, HIGH);
    digitalWrite(DATA_CLOCK_2, HIGH);
    delayMicroseconds(6);
  }

  /* These should always be HIGH (see gamefaqs) - if they're LOW, the controller is likely disconnected! */
  if (buttons_1[12] != HIGH && buttons_1[13] != HIGH && buttons_1[14] != HIGH && buttons_1[15] != HIGH) skip_j1 = true;
  if (buttons_2[12] != HIGH && buttons_2[13] != HIGH && buttons_2[14] != HIGH && buttons_2[15] != HIGH) skip_j2 = true;
  /** Set Joystick buttons based on SNES input 
      First three buttons are actual buttons
  **/
  for (int i = 0; i <= 3; i++) {
    if (!skip_j1) Joystick[0].setButton(button_map[i], !buttons_1[i]);
    if (!skip_j2) Joystick[1].setButton(button_map[i], !buttons_2[i]);
  }
  /** Set axes; "buttons" 4-7 are actually axes **/
  if (!skip_j1) {
    Joystick[0].setXAxis(127*(buttons_1[6]-buttons_1[7]));
    Joystick[0].setYAxis(127*(buttons_1[4]-buttons_1[5]));
  }
  if (!skip_j2) {
    Joystick[1].setXAxis(127*(buttons_2[6]-buttons_2[7]));
    Joystick[1].setYAxis(127*(buttons_2[4]-buttons_2[5]));
  }
  /** Set Joystick buttons based on SNES input
      Buttons 8-11 are buttons again
  **/
  for (int i = 8; i <= 11; i++) {
    if (!skip_j1) Joystick[0].setButton(button_map[i], !buttons_1[i]);
    if (!skip_j2) Joystick[1].setButton(button_map[i], !buttons_2[i]);
  }
}
