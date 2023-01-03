
/*
 * This is a Guitar Hero guitar on an ESP32
 * Is based on ESP32-BLE-Gamepad library's MultipleButtonsDebounce example
 * Uses Bounce2 and ESP32-BLE-Gamepad libraries
 * https://github.com/lemmingDev/ESP32-BLE-Gamepad
 * https://github.com/thomasfredericks/Bounce2
 * 
 * 
 * GPIO 4    start
 * GPIO 13   select
 * GPIO 16   dpad up
 * GPIO 17   dpad down
 * GPIO 18   dpad left
 * GPIO 19   dpad right
 * GPIO 21   green   x
 * GPIO 22   red     o
 * GPIO 23   yellow    square
 * GPIO 25   blue    triangle
 * GPIO 26   orange    L1
 * GPIO 27   ps button
 * GPIO 32   strum down
 * GPIO 33   strum up
 * 
 * GPIO 34   Whammy
 *
 * tilt      non-functional
 */
 
// Make button state changes available immediately
#define BOUNCE_WITH_PROMPT_DETECTION    

#include <Bounce2.h>      // https://github.com/thomasfredericks/Bounce2
#include <BleGamepad.h>   // https://github.com/lemmingDev/ESP32-BLE-Gamepad

#define debounceMS 3
#define numOfButtons 8
#define reportedNumOfButtons 13

#define strumUpPin 33 
#define strumDownPin 32

#define dpadUpPin 16
#define dpadDownPin 17
#define dpadLeftPin 18
#define dpadRightPin 19

#define whammyPin 34
//when whammyBar is pressed all the way down the read value is about 1000
#define whammyMin 1400

BleGamepad bleGamepad("Custom GH Guitar", "Markus", 100);

Bounce debouncers[numOfButtons];

Bounce strumUpBounce;
Bounce strumDownBounce;

Bounce dpadUpBounce;
Bounce dpadDownBounce;
Bounce dpadLeftBounce;
Bounce dpadRightBounce;

//order: green, red, yellow, blue, orange, start, select, PS button
byte buttonPins[numOfButtons] = { 21, 22, 23, 25, 26, 4, 13, 27};
byte buttonIDs[numOfButtons] = { 2, 3, 1, 4, 5, 10, 9, 13};

//raw whammy value
int whammyRead = 0;

//temporary dpad values
bool upState;
bool downState;
bool leftState;
bool rightState;

void setup()
{
  for (byte currentPinIndex = 0 ; currentPinIndex < numOfButtons ; currentPinIndex++)
  {
    debouncers[currentPinIndex] = 
      setupBouncer(buttonPins[currentPinIndex], INPUT_PULLUP, debounceMS);  
  }

  strumUpBounce = setupBouncer(strumUpPin, INPUT_PULLUP, debounceMS);
  strumDownBounce = setupBouncer(strumDownPin, INPUT_PULLUP, debounceMS);
  dpadUpBounce = setupBouncer(dpadUpPin, INPUT_PULLUP, debounceMS);
  dpadDownBounce = setupBouncer(dpadDownPin, INPUT_PULLUP, debounceMS);
  dpadLeftBounce = setupBouncer(dpadLeftPin, INPUT_PULLUP, debounceMS);
  dpadRightBounce = setupBouncer(dpadRightPin, INPUT_PULLUP, debounceMS);

  //enabled: buttons, hat switch, x, y, z, rZ
  //disabled: rX, rY, slider 1, slider 2, rudder, throttle, accelerator, brake, steering
  //order: buttons, hat switch, x, y, z, rZ, 
  //  rX, rY, slider 1, slider 2, rudder, throttle, accelerator, brake, steering
  bleGamepad.begin(reportedNumOfButtons, 1, true, true, true, true, 
    false, false, false, false, false, false, false, false, false);
  bleGamepad.setAutoReport(false);
  Serial.begin(115200);
}

void loop()
{
  if(bleGamepad.isConnected())
  {
    
    for (byte currentIndex = 0 ; currentIndex < numOfButtons ; currentIndex++)
    {
      debouncers[currentIndex].update();

      if (debouncers[currentIndex].fell())
      {
        bleGamepad.press(buttonIDs[currentIndex]);
        Serial.print("Button ");
        Serial.print(buttonIDs[currentIndex]);
        Serial.println(" pushed.");
      }
      else if (debouncers[currentIndex].rose())
      {
        bleGamepad.release(buttonIDs[currentIndex]);
        Serial.print("Button ");
        Serial.print(buttonIDs[currentIndex]);
        Serial.println(" released.");
      }
    } 

    bleGamepad.setHat(getHatState());

    //read 12bit analogue value and map it to 16 bit joystick range
    whammyRead = analogRead(whammyPin);

    //handle deadzone
    //upper deadzone doesn't need to be handled as ESP32's adc already has big deadzones
    if(whammyRead < whammyMin){
      whammyRead = whammyMin;
    }

    //lower range is -1 because map doesn't round correctly
    bleGamepad.setZ(map( whammyRead, whammyMin, 4095, 32737, -1 ));
    //bleGamepad.setRZ(0);
    
    bleGamepad.sendReport();
  }
}

Bounce setupBouncer(uint8_t pin, uint8_t pinM, uint16_t intervalMS)
{
  pinMode(pin, pinM);
  
  Bounce bounce = Bounce();
  bounce.attach(pin);
  bounce.interval(intervalMS);

  return bounce;
}

signed char getHatState(){
  
  strumUpBounce.update();
  strumDownBounce.update();
  dpadUpBounce.update();
  dpadDownBounce.update();
  dpadLeftBounce.update();
  dpadRightBounce.update();
  
  upState = !dpadUpBounce.read() || !strumUpBounce.read();
  downState = !dpadDownBounce.read() || !strumDownBounce.read();
  leftState = !dpadLeftBounce.read();
  rightState = !dpadRightBounce.read();

  // reproduces original GH guitar behavior
  if(upState && downState){
    return HAT_CENTERED;
  }
  
  if(upState){
    if(leftState){
      return HAT_UP_LEFT;
    }
    else if(rightState){
      return HAT_UP_RIGHT;
    }
    else{
      return HAT_UP;
    }
  }
  else if(downState){
    if(leftState){
      return HAT_DOWN_LEFT;
    }
    else if(rightState){
      return HAT_DOWN_RIGHT;
    }
    else{
      return HAT_DOWN;
    }
  }
  else if(leftState){
    return HAT_LEFT;
  }
  else if(rightState){
    return HAT_RIGHT;
  }
  
  return HAT_CENTERED;
}
