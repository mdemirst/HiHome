#include <SD.h>

// Define LED pin numbers
#define LED_GREEN 9
#define LED_YELLOW 10
#define LED_RED 11
#define LED_X 11

// Door alarm lamp pin number
#define LAMP_PIN 13

// Motor PWM pin
// Motor is used for spraying water to wake you up
#define MOTOR_PWM 10

// Multi purpose button pins
#define BUTTON1 8
#define BUTTON2 12

// Door alarm sensor input pin
#define DOOR_ALARM_PIN A0

// Siren buzzer output pin 
#define SIREN_PIN 11

// How long door alarm siren will ring
// This will be adjusted also using potentiometer.
// This value is the maximum value
#define SIREN_DURATION 15

// Pot input for adjusting siren duration
#define POT  A5

// Pot value
int  sirenDurationPot = 0;

// counter value to measure siren duration
int  sirenCount = 0;

// Siren actively ringing or not
bool sirenActive = false;

// Wake-up alarm active or not
// Set by buttons
bool alarmsActive = false;

// Snooze button pressed or not
bool snoozePressed = false;

// Alarm currently ringing or not
bool alarmStatus = false;


// Task management counters
long count_1ms = 0;
long count_10ms = 0;
long count_100ms = 0;
long count_1sec = 0;

// Email script is used for notifying the user
// for various situations such as when door alarm 
// is activated by suspicious activity
File emailScript;

// Email sent flag 
bool emailSent = false;
void setup() {
  Serial.begin(115200);
  
  pinMode(DOOR_ALARM_PIN, INPUT);
  digitalWrite(DOOR_ALARM_PIN, LOW);
  
  pinMode(SIREN_PIN, OUTPUT);
  pinMode(SIREN_PIN, HIGH);
  
  pinMode(POT, INPUT);
  
  pinMode(2,INPUT);
  pinMode(13,OUTPUT);
  
  pinMode(LED_GREEN,OUTPUT);
  pinMode(LED_YELLOW,OUTPUT);
  pinMode(LED_RED,OUTPUT);
  pinMode(LED_X,OUTPUT);
  pinMode(LAMP_PIN,OUTPUT);
  
  
  pinMode(BUTTON1, INPUT);
  pinMode(BUTTON2, INPUT);
  digitalWrite(BUTTON1, HIGH);
  digitalWrite(BUTTON2, HIGH);
  
  
  // SD card will store the python email send script
  if(!SD.begin(4))
  {
    Serial.println("Mail script initialization failed!");
    return;
  }
  
  // we'll create python script in the code
  // so, delete the previous one
  if(SD.exists("emailScript.py"))
  {
    SD.remove("emailScript.py");
  }
}

// time and alarm buf is used for continously reading 
// current time and alarm time from the OS system.
char timeBuf[5];
char alarmBuf[6];

// Read alarm and current time is written to variables
int alarm_hour = 1;
int alarm_min = 46;
int curr_hour = 0;
int curr_min = 0;

// How long wake-up alarm will ring
int alarm_duration = 2;

// Email send script
bool sendEmail()
{
  emailScript = SD.open("emailScript.py", FILE_WRITE);
  
  if(emailScript)
  {
    emailScript.println("import smtplib");
    emailScript.println("try:");
    emailScript.println("    fromaddr = 'sender@gmail.com'");
    emailScript.println("    toaddrs = 'receiver@gmail.com'");
    emailScript.println("    msg = \"\\r\\n\".join([");  
    emailScript.println("    \"From: sender@gmail.com\",");  
    emailScript.println("    \"To: reveiver@gmail.com\",");  
    emailScript.println("    \"Subject: Door alarm, suspicious action is detected! \",");  
    emailScript.println("    ]\)");  
    emailScript.println("    username = 'sender@gmail.com'");  
    emailScript.println("    password = 'password'");  
    emailScript.println("    server = smtplib.SMTP\('smtp.gmail.com:587'\)");
    emailScript.println("    server.ehlo()");  
    emailScript.println("    server.starttls\(\)");  
    emailScript.println("    server.login\(username,password\)");  
    emailScript.println("    server.sendmail\(fromaddr, toaddrs, msg\)");  
    emailScript.println("    server.quit\(\)");  
    emailScript.println("    print 'send success'");  
    emailScript.println("except Exception,e:");  
    emailScript.println("    print Exception,'error: ',e ");  
    
    emailScript.close(); 
    
    delay(500);
    system("python /media/realroot/emailScript.py > /media/realroot/emails.txt");  
    
    /*
    // Email sent results are stored in emails.txt
    emailScript = SD.open("emails.txt");  
    if (emailScript) {  
      Serial.println("emails.txt");  
    
      // read from the file until there's nothing else in it:  
      while (emailScript.available()) {  
         Serial.write(emailScript.read());  
      }  
      // close the file:  
      emailScript.close();  
    } else {  
       // if the file didn't open, print an error:  
       Serial.println("error opening emails.txt");  
    }  
    delay(500);  */
  }
}

// Check wake-up alarm should be activated or not
// based on the current time and alarm time 
// read from OS system
bool checkAlarmStatus()
{
  
  if(alarmsActive == false)
    return false;
    
  int diff = ((curr_hour*60+curr_min)-(alarm_hour*60+alarm_min));
  if( diff >=0 && diff <= alarm_duration )
  {
    return true;
  }
  else
  {
    return false;     
  }
}

// This function read the current time from text file in the /home/root/ directory
// Simple solution to get real time.
void readTime()
{
  system("TZ=EEST-3 date +%H%M > /home/root/time.txt");  //get current time in the format- hours:minutes:secs
                                                     //and save in text file time.txt located in /home/root
  FILE *fp;
  fp = fopen("/home/root/time.txt", "r");
  fgets(timeBuf, 5, fp);
  fclose(fp);
 
  String timeStr = String(timeBuf);
  String hourStr = timeStr.substring(0,2);
  String minStr = timeStr.substring(2,4); 
  
  curr_hour = hourStr.toInt();
  curr_min = minStr.toInt();
  /*
  Serial.print("Time:");
  Serial.print(curr_hour);
  Serial.print(":");
  Serial.println(curr_min);
  */
}

// This function reads the alarm time from text file in the /home/root/ directory
// Simple solution to write to alarm.txt remotely is to use shh client application
// in the android phone(ex: echo 19:19 > alarm.txt)
void readAlarmTime()
{
  
  FILE *fp;
  fp = fopen("/home/root/alarm.txt", "r");
  fgets(alarmBuf, 6, fp);
  fclose(fp);
 
  String timeStr = String(alarmBuf);
  String hourStr = timeStr.substring(0,2);
  String minStr = timeStr.substring(3,5); 
  
  alarm_hour = hourStr.toInt();
  alarm_min = minStr.toInt();
  /*
  Serial.print("Time:");
  Serial.print(alarm_hour);
  Serial.print(":");
  Serial.println(alarm_min);*/
  
}

// If wake-up alarm is activated and sypay value is true 
// motor that starts spraying water gradually 
// at the beginning pwm is so little that motor cannot
// spray water but sound like it will spray in a minute.
// That is a cool motivation to wake up and press snooze button 
// before it sprays
void sprayWaterGrad(bool spray)
{
  static int sprayAmount = 0;
  static long spreyStarted = 0;
  
  if(spray == false)
  {
    sprayAmount = 0;
    spreyStarted = millis();
    analogWrite(MOTOR_PWM, 0);
  }
  else
  {
    int pwm = 255*(((millis()-spreyStarted)/1000.0)/(alarm_duration*60));
    if(pwm > 255) pwm = 255;
    analogWrite(MOTOR_PWM, pwm); 
  }
}

// Door alarm sensor is one of the simple car-park sensors
// I modified it a little so that I get above 3.3V when alarm
// is activated.
void checkDoorAlarm()
{
  if(digitalRead(DOOR_ALARM_PIN) == HIGH)
  {
    sirenActive = true;
  }
  
  
  // If sensor is activated
  // ring the siren and turn on the lamp
  if(sirenActive)
  {
    digitalWrite(SIREN_PIN,HIGH);
    digitalWrite(LAMP_PIN,HIGH);
  }
  else
  {
    digitalWrite(SIREN_PIN,LOW);
    digitalWrite(LAMP_PIN,LOW);
  }

}

void loop() {
  
  
  long currCount = millis();
  
  //1 millisecond period call
  if((currCount-count_1ms) >= 1)
  {
    // Button 1 is used for turning on and off wake-up alarm
    if(digitalRead(BUTTON1) == LOW)
    {
      alarmsActive = !alarmsActive;
      // Green led shows alarm is active
      digitalWrite(LED_GREEN, alarmsActive);
      while(digitalRead(BUTTON1) == LOW){};
    }
    
    // Button 2 is used for snooze
    if(digitalRead(BUTTON2) == LOW && snoozePressed == false)
    {
      snoozePressed = true;
    }
    
    //If wake-up alarm is not ringing then clear snooze flag
    if(snoozePressed == true && alarmStatus == false)
    {
      snoozePressed = false; 
    } 
  
    count_1ms = currCount;
  }
  
  //10 millisecond period call
  if((currCount-count_10ms) >= 10)
  {
    // Door alarm sensor is checked
    checkDoorAlarm();
    
    count_10ms = currCount;
  }
  
  //100 millisecond period call
  if((currCount-count_100ms) >= 100)
  {
    count_100ms = currCount;
  }
  
  //1000 millisecond period call
  if((currCount-count_1sec) >= 1000)
  {
    
    // update siren duration from potantiometer
    sirenDurationPot = SIREN_DURATION*(analogRead(POT)/1024);
    
    // Read current time and alarm time
    readTime();
    readAlarmTime();
    
    // check if wake-up alarm should be activated or not
    alarmStatus = checkAlarmStatus();
    if(alarmStatus == true && snoozePressed == false)
    {
      //Serial.println("Alarming");
      
      sprayWaterGrad(true);
    }
    else
    {
      //Serial.println("Not Alarming");
      
      sprayWaterGrad(false);
    }
    
    // Let siren to ring at least some duration even
    // it's activated for a little.
    // If siren is active, then sent email to user for one time.
    if(sirenActive)
    {
      sirenCount++;
      if(sirenCount >= sirenDurationPot)
      {
        sirenCount = 0;
        sirenActive = false;
      }
      
      if(emailSent == false)
      {
        sendEmail();
        emailSent = true;
      }
    }
    else
    {
      emailSent = false;
    }
  
    count_1sec = currCount;
  }
  
  // wait a little before the next loop
  delay(10);
}
