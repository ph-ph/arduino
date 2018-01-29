/*
SparkFun Inventor’s Kit
Circuit 4A-HelloWorld

The LCD will display the words "Hello World" and show how many milliseconds have passed since
the RedBoard was last reset.

This sketch was written by SparkFun Electronics, with lots of help from the Arduino community.
This code is completely free for any use.

View circuit diagram and instructions at: https://learn.sparkfun.com/tutorials/sik-experiment-guide-for-arduino---v4
Download drawings and code at: -- GITHUB LINK --
*/

#include <LiquidCrystal.h>          //the liquid crystal libarry contains commands for printing to the display

#define TIMER_INTERVAL 25*60 // standard pomodoro interval time, in seconds
#define GREEN_LED_PIN 7
#define GREEN_BUTTON_PIN 6
#define RED_BUTTON_PIN 5
#define RED_LED_PIN 4

class PomodoroTimer {
public:
  PomodoroTimer() {
    _isInProgress = false;
    _timerRunning = false;
    _timeToRun = TIMER_INTERVAL; 
  }
  
  void start() {
    _isInProgress = true;
    _timerRunning = true;
    _timerStartMs = millis();
  }

  void pause() {
    _timerRunning = false;
    _timeToRun = _timeToRun - (millis() - _timerStartMs)/1000;    
  }

  void reset() {
    _isInProgress = false;
    _timerRunning = false;
    _timeToRun = TIMER_INTERVAL;
  }

  bool isInProgress() const {
    return _isInProgress;
  }

  bool isRunning() const {
    return _timerRunning;
  }

  long secondsRemaining() const {
    if (isRunning()) {
      return _timeToRun - (millis() - _timerStartMs)/1000;
    } else {
      return _timeToRun;
    }
  }
  
private:
  bool _timerRunning = false;
  bool _isInProgress = false;
  long _timeToRun = TIMER_INTERVAL;
  unsigned long _timerStartMs = 0;
};

// flashes LED if the process is on, does nothing otherwise
class ProcessIndicator {
public:
  ProcessIndicator(int pinId) {
    _isOn = false;
    _pinId = pinId;
  }

  void turnOn() {
    _isOn = true;
  }

  void turnOff() {
    _isOn = false;
  }

  void update() {
    if (_isOn) {
      digitalWrite(_pinId, ((millis()/200) % 2 == 0) ? HIGH : LOW);
    } else {
      digitalWrite(_pinId, LOW);
    }    
  }
  
private:
  int _pinId;
  bool _isOn;
};

LiquidCrystal lcd(13, 12, 11, 10, 9, 8);   // tell the RedBoard what pins are connected to the display
float prevTemp = 0.0;
bool redLedFlashing = false;
PomodoroTimer timer;
ProcessIndicator redIndicator(RED_LED_PIN);

void setup() {                     
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_BUTTON_PIN, INPUT_PULLUP);
  pinMode(RED_BUTTON_PIN, INPUT_PULLUP);
  lcd.begin(16, 2);                 //tell the lcd library that we are using a display that is 16 characters wide and 2 characters high
  lcd.clear();                      //clear the display
  Serial.begin(9600);
}

void loop() {
  lcd.setCursor(0, 0);

  // Check if we need to switch on the red led
  String command = readCommandFromSerial();
  if (command == "red_on") {
    redIndicator.turnOn();
  } else if (command == "red_off") {
    redIndicator.turnOff();
  }

  redIndicator.update();

  if (buttonIsPressed(RED_BUTTON_PIN)) {
    timer.reset();
    erase_timer_block();
    delay(500);
    return;
  }

  if (buttonIsPressed(GREEN_BUTTON_PIN)) {
    Serial.println("pressed green button");
    if (timer.isRunning()) {
      Serial.println("pausing timer");
      timer.pause();
    } else {
      Serial.println("starting timer");
      timer.start();
      lcd.clear();
    }
    delay(500);
    return;
  } 
  
  if (timer.isInProgress()) {
    long timeRemaining = timer.secondsRemaining();
//    Serial.println("timer is in progress");
//    Serial.println(timeRemaining);
    if (timeRemaining >= 0) {
      print_timer_block(timeRemaining);         //print the number of seconds that have passed since the last reset
    } else if (timeRemaining > -10) { //flash led
      digitalWrite(GREEN_LED_PIN, ((millis()/200) % 2 == 0) ? HIGH : LOW);
    } else {
      timer.reset();
      digitalWrite(GREEN_LED_PIN, LOW);
    }      
  } else {
    erase_timer_block();
    display_temperature();      
  }
} 

bool buttonIsPressed(int pin) {
  return digitalRead(pin) == LOW;
}

String readCommandFromSerial() {
  if (Serial.available() > 0) {
    return Serial.readStringUntil(' ');
  }
  return "";
}

void print_timer_block(int timeInSeconds) {
  char buf[3];
  lcd.print("> ");
  sprintf(buf, "%02d", timeInSeconds/60);
  lcd.print(buf);
  lcd.print(":");
  sprintf(buf, "%02d", timeInSeconds % 60);
  lcd.print(buf);
}

void erase_timer_block() {
  lcd.print("       ");
}

void display_temperature() {
  float voltage = analogRead(A0) * 0.004882814;   //convert the analog reading, which varies from 0 to 1023, back to a voltage value from 0-5 volts
  float degreesC = (voltage - 0.5) * 100.0;       //convert the voltage to a temperature in degrees Celsius
  if (fabs(degreesC - prevTemp) > 1.5) {
    prevTemp = degreesC;
    delay(100);
  }
  lcd.setCursor(7, 0);
  lcd.print("T: ");
  lcd.print(prevTemp);
  lcd.print("C");  
}

