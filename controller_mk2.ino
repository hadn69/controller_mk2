#include <ACS712.h>
#include <CytronMotorDriver.h>
#include <LedControl.h>
#include <LedPrint.h>
#include <LedPrintJustifiable.h>

//Global Variables
int throttle = A5;
int mom = A4;
int tempSpeed = 0;
int direction = 0;
int lastDir = 0;
double currentArray[20];

//devices
ACS712 sensor(ACS712_30A, A15);
CytronMD motor(PWM_DIR, 3, 2); // PWM = Pin 3, DIR = Pin 2.
LedPrintJustifiable myLed = LedPrintJustifiable(
    22, // DATA PIN
    24, // CLOCK PIN
    26, // CS PIN
    8,  // NUMBER OF DIGITS
    1   // Orientation 0/1, if it looks backwards try the other
);
LedPrintJustifiable displayA;
LedPrintJustifiable displayB;
LedPrintJustifiable displayA2;
LedPrintJustifiable displayB2;
//end devices

void setup()
{
  //house keeping
  pinMode(direction, INPUT_PULLUP);
  myLed.setIntensity(5);
  sensor.calibrate();
  for (int i = 0; i < (sizeof(currentArray) / sizeof(currentArray[0])); i++)
  {
    currentArray[i] = 0;
  }

  //start, # of digits
  displayA = myLed.getSubDisplay(0, 3);
  displayA2 = myLed.getSubDisplay(3, 1);
  displayB = myLed.getSubDisplay(5, 3);
  displayB2 = myLed.getSubDisplay(4, 1);

  //magic numbers
  displayA.justify(-1);
  displayB.justify(1);
}

void loop()
{
  //temp sensor reads
  double current = sensor.getCurrentDC();
  int throttlePosition = analogRead(throttle);
  int directionPosition = digitalRead(direction);

  //execute
  run(throttlePosition, directionPosition);
  display(tempSpeed, directionPosition, current);
}

void run(int speed, int dir)
{
  //motor control
  if (abs(tempSpeed - speed) > 30)
  {
    //get major speed change
    tempSpeed = speed;
  }
  else if (speed < 30)
  {
    //reasonable 0 point?
    tempSpeed = 0;
    speed = 0;
  }
  else
  {
    //maintain last speed
    speed = tempSpeed;
  }
  //adjust range
  int temp = (speed / 4);
  // why no fucking .dir you dumb library
  if (dir)
  {
    motor.setSpeed(temp);
  }
  else
  {
    temp = temp * -1;
    motor.setSpeed(temp);
  }
}

void display(int speed, int dir, double I)
{
  //fix ghosting maybe?
  myLed.clearEntireDisplay();
  displayA.clear();
  displayB.clear();

  //speed shit
  if (speed > 1000)
  {
    speed = 1000;
  }
  if (!dir)
  {
    displayB2.println("-");
  }
  else
  {
    displayB2.clear();
  }

  displayB.println(speed / 10);

  //current shit
  double tempCurrent = 0.0;
  //current averaging
  for (int i = 0; i < (sizeof(currentArray) / sizeof(currentArray[0])); i++)
  {
    if (i == 19)
    {
      currentArray[i] = I;
    }
    else
    {
      currentArray[i] = currentArray[i + 1];
    }
  }
  //get average current of last 20 cycles
  for (int i = 0; i < (sizeof(currentArray) / sizeof(currentArray[0])); i++)
  {
    tempCurrent = tempCurrent + currentArray[i];
  }
  //magic
  tempCurrent = tempCurrent / 20;
  //formating and display
  if (tempCurrent < 0.2)
  {
    tempCurrent = 0.0;
  }
  displayA.print(tempCurrent, 1);
  displayA2.println("A");
}