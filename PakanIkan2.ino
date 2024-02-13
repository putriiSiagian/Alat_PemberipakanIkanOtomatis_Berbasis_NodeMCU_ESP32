#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32_Servo.h>
#include <RTClib.h>

RTC_DS1307 rtc;
Servo servo;
LiquidCrystal_I2C lcd(0x27, 16, 2); // Set the LCD pins

// Feed schedule
const int feedTimes[] = {5, 10, 20};      // Feed times in hours
const float feedMinutes[] = {0, 32, 0};    // Feed minutes
const int numFeedTimes = sizeof(feedTimes) / sizeof(feedTimes[0]);

// Ultrasonic sensor pins
const int trigPin = 19;
const int echoPin = 18;

// Buzzer pin
const int buzzerPin = 2;

// Servo operation duration
const int servoOnDuration = 500; // 0.5 seconds
const int feedDuration = 50000;    // 1 minute

unsigned long startTime = 0; // Variable to store servo start time

void setup()
{
  Serial.begin(9600);
  Wire.begin();

  // RTC setup
  rtc.begin();

  if (!rtc.isrunning())
  {
    Serial.println("RTC is not running!");
    // Set the RTC time to the laptop's time if not running
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Synchronize RTC with laptop's time
  rtc.adjust(DateTime(__DATE__, __TIME__));

  // Servo setup
  servo.attach(14); // Set the servo pin

  // LCD display setup
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Fish Feeder");

  // Ultrasonic sensor setup
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Buzzer setup
  pinMode(buzzerPin, OUTPUT);

  Serial.println("Fish Feeder started");
}

void loop()
{
  // Get current time from RTC
  DateTime now = rtc.now();
  int currentHour = now.hour();
  int currentMinute = now.minute();

  // Check if it's time to feed
  for (int i = 0; i < numFeedTimes; i++)
  {
    if (currentHour == feedTimes[i] && currentMinute == feedMinutes[i] && now.second() == 0)
    {
      updateLCDTime(now);
      feedFish();
      updateLCDTime(now);
    }
  }

  // Update LCD display with RTC time every second
  updateLCDTime(now);

  // Check fish food level
  int foodLevel = measureFoodLevel();
  if (foodLevel <= 5)
  {
    buzzerOn();
    delay(1000);
    buzzerOff();
  }

  // Delay for 1 second
  delay(1000);
}

void updateLCDTime(DateTime time)
{
  lcd.setCursor(0, 1);
  lcd.print(time.day(), DEC);
  lcd.print("/");
  lcd.print(time.month(), DEC);
  lcd.print("/");
  lcd.print(time.year() % 100, DEC);
  lcd.print(" ");
  lcd.print(time.hour(), DEC);
  lcd.print(":");
  lcd.print(time.minute(), DEC);
  lcd.print(":");
  if (time.second() < 10) {
    lcd.print("0");  // Add leading zero for seconds less than 10
  }
  lcd.print(time.second(), DEC);
}

void feedFish()
{
  // Turn on servo
  servo.write(90);
  startTime = millis(); // Record the start time of servo operation

  // Display message on LCD display
  lcd.setCursor(0, 0);
  lcd.print("Feed at ");
  lcd.print(rtc.now().hour(), DEC);
  lcd.print(":");
  lcd.print(rtc.now().minute(), DEC);
  lcd.print(":");
  lcd.print(rtc.now().second(), DEC);

  // Keep the servo on for the specified duration
  while (millis() - startTime < servoOnDuration)
  {
    // Allow other tasks to execute while the servo is active
    // For example, updating the LCD display or checking food level
    // No additional delay is needed here
  }

  servo.write(0); // Turn off servo
  delay(feedDuration - servoOnDuration); // Delay remaining time
}

int measureFoodLevel()
{
  // Send ultrasonic pulse
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read echo duration
  unsigned long duration = pulseIn(echoPin, HIGH);

  // Calculate distance in cm
  int distance = duration * 0.034 / 2;

  return distance;
}

void buzzerOn()
{
  digitalWrite(buzzerPin, HIGH);
}

void buzzerOff()
{
  digitalWrite(buzzerPin, LOW);
}
