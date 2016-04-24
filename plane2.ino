/**
*  This code is used to control an HC-SR04 ultrasonic sensor,
*  read from a servo signal, and output a modified servo signal
*  that allows a plane to pull up automatically once it nears
*  the ground.
*
*  Requires: 
*    1. HC-SR04 on pins 2_2 (trigger) and 2_1 (echo)
*    2. HCF4051 in 2-channel mode with channel 0 for the receiver and channel 1 on pin 1_5
*    3. TowerPro 9g servo or any other compatible 20ms-period servo, paired with a compatible receiver
*    4. S8050 for logic level conversion (3.3V to 5V)
*    5. ...and a lot of pull resistors. Don't worry about it.
*
*   System created by the folks at Training Wings (not yet trademarked) UVA.
*   Anant Kharkar, Leonard Ramsey, Ailec Wu, Alan Zhang
**/
int potpin = P1_0;
int servopin = P1_5;
int trigpin = P2_2;
int echopin = P2_1;
int val;
int newval;
volatile boolean servoflag = 0;
volatile boolean lockup = 0;
volatile int servocount = 0;
volatile boolean validtime = 0;
long curtime = micros();
volatile long starttime = micros();
volatile long stoptime = micros();
//volatile long startping = micros();
//volatile long stopping = micros();
volatile int pingtime = 0;

void startservo() {
  servoflag = 1;
  validtime = 0;
  starttime = micros();
  //digitalWrite(P1_6, HIGH);
}
void stoptick() {
  stoptime = micros();
  validtime = 1;
  //digitalWrite(P1_6, LOW);
}
/*void readecho() {
  stopping = micros();
}*/
void setup()
{
  // put your setup code here, to run once:
  //Serial.begin(9600);
  //Serial.println("ACTIVE");
  pinMode(P1_0, OUTPUT);
  digitalWrite(P1_0, HIGH);
  pinMode(servopin, OUTPUT);
  pinMode(potpin, INPUT);
  pinMode(trigpin, OUTPUT);
  pinMode(echopin, INPUT);
  pinMode(P1_6, OUTPUT);
  digitalWrite(P1_6, LOW);
  pinMode(P2_3, INPUT_PULLDOWN);
  pinMode(P2_4, INPUT_PULLDOWN);
  attachInterrupt(P2_3, startservo, RISING);
  attachInterrupt(P2_4, stoptick, FALLING);
}

void setservo(int val) {
  digitalWrite(servopin, LOW);
  delayMicroseconds(val);
  digitalWrite(servopin, HIGH);
}

void loop()
{
  if (servoflag) { //it's time to run the servo
    setservo(1000); // 1ms = pull up
    servoflag = 0; //just finished running the servo
    servocount++;
    if (servocount == 5) {//get ready to ping out
      digitalWrite(trigpin, LOW);
      delayMicroseconds(2);
      digitalWrite(trigpin, HIGH);
      delayMicroseconds(5);
      digitalWrite(trigpin, LOW);
      servocount = 0;
      if (lockup == 0) {
        pingtime = pulseIn(echopin, HIGH, 15000);
        if (pingtime == 0) { //lock-up, reset required
          lockup = 1;
        }
      } else { //do the reset
          pinMode(echopin, OUTPUT);
          digitalWrite(echopin, LOW);
          delayMicroseconds(17000);
          pinMode(echopin, INPUT);
          lockup = 0;
      }
    }
  }
  if (validtime) { //Check to see if the operator switch has been flipped and we need to pull up
    //Ping threshold set to 2000, about 1 ft, for demonstration purposes
    if (stoptime - starttime > 1250 && pingtime > 0 && pingtime < 2000) digitalWrite(P1_6, HIGH);
    //if (pingtime > 0 && pingtime < 2000) digitalWrite(P1_6, HIGH);
    else digitalWrite(P1_6, LOW);
  }
}
