#include <MicroView.h>
#include <Servo.h>

// This pin is for "up" button
const byte pin_up_button = 5;
// This pin is for "down"button
const byte pin_down_button = 6;
// Default gear
byte current_gear = 5;

// Pin for enable "tuning" mode 
// (during which we can change the "gears" array's values)
const byte pin_tuning_mode = 0;
const byte pin_tune_up = 1;
const byte pin_tune_down = 3;

// Flag for "tuning" mode
bool tuning = false;

byte gear_up_pressed, gear_down_pressed, tune_mode_pressed, tune_up_pressed, tune_down_pressed = 0;

// Class for interacting with rear durailleur's servo
Servo rear_durailleur_servo;
const byte pin_rear_durailleur_servo = 2;

const byte gears_count = 9;
// Defines, which servo angle corresponds to each gear
byte gears[gears_count] = {0, 20, 40, 60, 80, 100, 120, 140, 160};

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

  pinMode(pin_tuning_mode, INPUT);
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

  // Detection of shrt-press of the "gear up" button
  if (digitalRead(pin_up_button) == LOW)
    gear_up_pressed++;
  else
    gear_up_pressed = 0;

  // Detection of shrt-press of the "gear down" button
  if (digitalRead(pin_down_button) == LOW)
    gear_down_pressed++;
  else
    gear_down_pressed = 0;

  // If the "tuning mode" button has been pressed for 100 loops
  // (long press)
  if (tune_mode_pressed == 100)
  {
    if (!tuning)
      tuning = true;
    else
      tuning = false;
  }

  uView.setCursor(0, 20);
  if (tuning)
    uView.print("tune on");
  else
    uView.print("tune off");

  if (tuning && tune_up_pressed == 1 && gears[current_gear] < gears[current_gear + 1])
    gears[current_gear]++;

  if (tuning && tune_down_pressed == 1 && gears[current_gear] > gears[current_gear - 1])
    gears[current_gear]--;

  if (gear_up_pressed == 1 && current_gear < gears_count - 1)
    current_gear++;

  if (gear_down_pressed == 1 && current_gear > 0)
    current_gear--;

  // Set the servo to the current gear's angle
  rear_durailleur_servo.write(gears[current_gear]);

  // Display info on the screen
  uView.setCursor(0, 0);
  uView.print(gears[current_gear]);
  uView.setCursor(0, 10);
  uView.print(current_gear);

  uView.display();

}
