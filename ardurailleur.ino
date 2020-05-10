#include <MicroView.h>
#include <Servo.h>
#include <EEPROM.h>

// This pin is for "up" button
const byte pin_up_button = 5;
// This pin is for "down"button
const byte pin_down_button = 6;
// Default gear
byte current_gear = 4;

// Pin for enable "tuning" mode 
// (during which we can change the "gears" array's values)
const byte pin_tuning_mode = 0;
const byte pin_tune_up = 1;
const byte pin_tune_down = 3;

// This pin is connected with rear light
const byte pin_rear_light = A0;
// This pin is for button, which will handle rear light
const byte pin_rear_light_button = A5;
bool rear_light_button_pressed = false;

// Flag for "tuning" mode
bool tuning = false;

byte gear_up_pressed, gear_down_pressed, tune_mode_pressed, tune_up_pressed, tune_down_pressed = 0;

// Class for interacting with rear durailleur's servo
Servo rear_durailleur_servo;
const byte pin_rear_durailleur_servo = 2;

const byte gears_count = 9;
// Defines, which servo angle corresponds to each gear
byte gears_angles[gears_count] = {20, 40, 60, 80, 100, 120, 140, 160, 180};
byte current_angle;

// Allow me to intriduce the "Overshifting" - it's the future of bicycle shifting!
// When you switch to the next gear, the durailleur goes over it for an overshift_timeout 
// (and after it back to the target gear) - 
// and this will surely shifts you gear even in cases of a large transmission wear or dirt!
bool overshifting_enabled = true;
bool overshift_up_in_process = false, overshift_down_in_process = false;
byte overshift_timer, overshift_angle;
byte overshift_timeout = 30;

/*---------------------------------------------------------------*/

void setup()
{
  // Start MicroView screen
  uView.begin();

  // Define servo's pin
  rear_durailleur_servo.attach(pin_rear_durailleur_servo);

  // Configuring pins as inputs and enabling pull-up resistors
  pinMode(pin_up_button, INPUT); 
  digitalWrite(pin_up_button, HIGH);

  pinMode(pin_down_button, INPUT); 
  digitalWrite(pin_down_button, HIGH);

  pinMode(pin_tune_up, INPUT); 
  digitalWrite(pin_tune_up, HIGH);

  pinMode(pin_tuning_mode, INPUT);

  // Configure this pin as input because rear light needs conection to ground (digital low) 
  // to be switched and I don't know what will be if connect it to 3.3V (digital high)
  pinMode(pin_rear_light, OUTPUT);
  pinMode(pin_rear_light_button, INPUT);

  byte g;
  // Reading stored gears angles from EEPROMovershift_down
  for(byte i = 0; i < gears_count; i++)
  {
    g = EEPROM.read(i);
    if (g != 255) // "Virgin" EEPROM's byte value is 255, so we don't need it
      gears_angles[i] = g;      
  }
}

void save_to_EEPROM()
{
  for(byte i = 0; i < gears_count; i++)
  {
    uView.clear(PAGE);
    uView.setCursor(0, 0);
    uView.print("Write EEPROM...");
    uView.setCursor(0, 10);
    uView.print("Gear "); uView.print(i + 1); uView.print(" val "); uView.print(gears_angles[i]);
    uView.display();
    // "Update" procedure is more sparing for EEPROM because it doesn't write value if it's the same
    EEPROM.update(i, gears_angles[i]);
  }
}

// Internal voltmeter reading
long readVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
     ADMUX = _BV(MUX5) | _BV(MUX0) ;
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif  
 
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring
 
  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH; // unlocks both
 
  long result = (high<<8) | low;
 
  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result; // Vcc in millivolts
}

void loop()
{
  uView.clear(PAGE);

  if (digitalRead(pin_tuning_mode) == LOW)
    tune_mode_pressed++;
  else
    tune_mode_pressed = 0;

  if (digitalRead(pin_tune_up) == LOW)
    tune_up_pressed++;
  else
    tune_up_pressed = 0;

  if (digitalRead(pin_tune_down) == LOW)
    tune_down_pressed++;
  else
    tune_down_pressed = 0;

  // Detection of short-press of the "gear up" button
  if (digitalRead(pin_up_button) == LOW)
    gear_up_pressed++;
  else
    gear_up_pressed = 0;

  // Detection of shrt-press of the "gear down" button
  if (digitalRead(pin_down_button) == LOW)
    gear_down_pressed++;
  else
    gear_down_pressed = 0;

  // Detection of rear light button short-pressing
  if (digitalRead(pin_rear_light_button) == LOW)
  {
    // State change detected
    if (!rear_light_button_pressed)
    {
      // I don't know what will be if send to rear light HIGH signal,
      // so we will just change this pin from input to output with low (ground) signal 
      // on each button pressing
      pinMode(pin_rear_light, OUTPUT);
      digitalWrite(pin_rear_light, LOW);
      rear_light_button_pressed = true;
    }
  }
  else // Detection of button release
  {
    // State change detected
    if (rear_light_button_pressed)
    {
      pinMode(pin_rear_light, INPUT);
      rear_light_button_pressed = false;
    }
  }

  // If the "tuning mode" button has been pressed for 100 loops
  // (long press)
  if (tune_mode_pressed == 100)
  {
    if (!tuning)
      tuning = true;
    else
    {
      tuning = false;
      // Saving new values to EEPROM
      save_to_EEPROM();
    }
  }

  uView.setCursor(0, 20);
  if (tuning)
    uView.print("tune: on");
  else
    uView.print("tune: off");

  if (tuning && tune_up_pressed == 1 && (gears_angles[current_gear] < gears_angles[current_gear + 1] || current_gear == gears_count - 1))
    gears_angles[current_gear]++;

  if (tuning && tune_down_pressed == 1 && (gears_angles[current_gear] > gears_angles[current_gear - 1] || current_gear == 0))
    gears_angles[current_gear]--;

  if (gear_up_pressed == 1 && current_gear < gears_count - 1)
  {
    current_gear++;
    
    if (overshifting_enabled)
    {
      overshift_up_in_process = true;
      overshift_timer = 0;
    }
  }

  if (gear_down_pressed == 1 && current_gear > 0)
  {
    current_gear--;
    
    if (overshifting_enabled)
    {
      overshift_down_in_process = true;
      overshift_timer = 0;
    }
  }

  if (!overshifting_enabled)
      // If there is no overshifting, just switch to the next gear
      current_angle = gears_angles[current_gear];
  else
  {
    if (overshift_up_in_process || overshift_down_in_process)
    {
      if (overshift_timer < overshift_timeout)
      {
        overshift_timer++;

        if (overshift_up_in_process)
          if (current_gear != gears_count - 1) // Go to the middle from current to next
            current_angle = gears_angles[current_gear] + (gears_angles[current_gear + 1] - gears_angles[current_gear]) / 2;
          else // If we switched to the last gear
            // Let's try to go over border
            current_angle = gears_angles[current_gear] + 10;

        if (overshift_down_in_process)
          if (current_gear != 0)
            current_angle = gears_angles[current_gear] - (gears_angles[current_gear] - gears_angles[current_gear - 1]) / 2;
          else // If we switched to the first gear
            // Let's try to go over border
            current_angle = 0;
      }
      else
      { // If overshift timer is out, switch to the target gear
        overshift_up_in_process = false;
        overshift_down_in_process = false;
        current_angle = gears_angles[current_gear];
      }
    }
    else // Hold current gear
      current_angle = gears_angles[current_gear];
  }

  // Set the servo to the current gear's angle
  rear_durailleur_servo.write(current_angle);

  // Display info on the screen
  uView.setCursor(0, 0);
  uView.print("gear: "); uView.print(current_gear + 1);
  uView.setCursor(0, 10);
  uView.print("angle: "); uView.print(current_angle);
  uView.setCursor(0, 30);
  uView.print("uV: "); uView.print(readVcc());
  uView.display();
}
