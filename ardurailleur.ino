#include <MicroView.h>
#include <Servo.h>

int pin = 6;
int pin2 = 5;
int g = 0;

int pin_mode = 0;
int pin_tune_up = 1;
int pin_tune_down = 3;

bool tuning = false;

int plus, minus, tune, tune_up, tune_down = 0;
Servo myservo;

int gears[9] = {0, 20, 40, 60, 80, 100, 120, 140, 160};

void setup()
{
  uView.begin();

  myservo.attach(2);
  pinMode(pin, INPUT); //конфигурируем пин как выход ( к нему подключен светодиод)
  digitalWrite(pin, HIGH); //включаем внутренний pull-up резистор

  pinMode(pin2, INPUT); //конфигурируем пин как выход ( к нему подключен светодиод)
  digitalWrite(pin2, HIGH); //включаем внутренний pull-up резистор

  pinMode(pin_mode, INPUT);
}

void loop()
{
  uView.clear(PAGE);

  uView.setCursor(0, 20);

  if (digitalRead(pin_mode) == LOW)
    tune++;
  else
    tune = 0;

  if (digitalRead(pin_tune_up) == LOW)
    tune_up++;
  else
    tune_up = 0;

  if (digitalRead(pin_tune_down) == LOW)
    tune_down++;
  else
    tune_down = 0;

  if (digitalRead(pin2) == LOW)
    plus++;
  else
    plus = 0;

  if (digitalRead(pin) == LOW)
    minus++;
  else
    minus = 0;

  if (tune == 100)
  {
    if (!tuning)
      tuning = true;
    else
      tuning = false;
  }

  if (tuning)
    uView.print("tune on");
  else
    uView.print("tune off");

  if (tuning && tune_up == 1 && gears[g] < gears[g + 1])
    gears[g]++;

  if (tuning && tune_down == 1 && gears[g] > gears[g - 1])
    gears[g]--;

  if (plus == 1 && g < 8)
    g++;

  if (minus == 1 && g > 0)
    g--;

  // устанавливаем сервопривод в крайнее правое положение
  myservo.write(gears[g]);
  uView.setCursor(0, 0);
  uView.print(gears[g]);
  uView.setCursor(0, 10);
  uView.print(g);

  uView.display();

}
