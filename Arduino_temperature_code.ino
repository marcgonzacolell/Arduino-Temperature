
#define CPU_FREQ 16000000L  // cpu clock
#define PRESCALER 1024      // cpu prescaler

const unsigned long TICKS_PER_SEC = (CPU_FREQ / PRESCALER);             // base for calculation of compare value for output compare register OCR1A
const unsigned short INTRPT_INTERVALL = 1;                              // interrupt every 0.25 seconds
const unsigned short CMP_VALUE = INTRPT_INTERVALL * TICKS_PER_SEC - 1;  // compare value for OCR1A: = amount of ticks in an interrupt intervall

volatile unsigned long counter = 0;
volatile unsigned long cmp = 0;

// pin numbers corresponding to signals on the TC Lab Shield
const int pinT1 = 0;   // T1
const int pinT2 = 2;   // T2
const int pinQ1 = 3;   // Q1
const int pinQ2 = 5;   // Q2
const int pinLED = 9;  // LED

float Q1 = 0;  // value written to Q1 pin
float Q2 = 0;  // value written to Q2 pin

//Control parameters
static float error = 0;      //error variable (ºC), usually, error = reference-measurement
static float u = 0;          //control action (W) from 0 to 1, this is the control output
static float err_ant = 0;    //control action (W) from 0 to 1, this is the control output
static float u_ant = 0;      //control action (W) from 0 to 1, this is the control output
static float u_dig = 0;      //PWM control action (W) from 0 to 255.
static float setpoint = 42;  //Reference setpoint (ºC), this value can be changed by the user to set a new reference
//Control constants
static float ki = 0;  //integral constant, you must update its initialization value.
static float kp = 0;  //proportional constant, you must update its initialization value.
//**********************************************************************
//Define here new control parameters if they are required in your design
//**********************************************************************
//previous results
static float integral = 0;    //result of the integral action (W)
static float integral_0 = 0;  //previous value of the integral action (W)
//**********************************************************************
//Define here new auxiliar control variables
//**********************************************************************

void setup() {
  analogReference(EXTERNAL);  // The adc uses an external reference
  Serial.begin(115200);       // Communications frequency: 115200 bauds

  // The code does not start until receives a serial communication
  //while (!Serial) {
  ;  // wait for serial port to connect.
  //}

  //Set an interrupt with a frequency of 4 Hz (i.e., Ts = 0.25 s)
  cli();                   // disable global interrupts
  TCNT1 = 0;               // delete timer counter register
  TCCR1A = 0;              // delete TCCR1A-Registers
  TCCR1B = 0;              // delete TCCR1B-Registers
  TCCR1B |= (1 << WGM12);  // CTC-Mode (Waveform Generation Mode): resets TCNT1 to0 after interrupt
  //TCCR1B |= (1 << CS12) | (1 << CS10); // set prescaler to 1024: CS10 und CS12 (Clock Select)
  TCCR1B |= (1 << CS12);    // set prescaler to 256:  CS12 (Clock Select)
  OCR1A = CMP_VALUE;        // set compare value
  TIMSK1 |= (1 << OCIE1A);  // enable interrupt: set output compare interrupt enable for 1A
  sei();                    // enable global interrupts
}

// Interrupt Service Routine for Timer 1, here is where the control action is calculated
ISR(TIMER1_COMPA_vect) {

  float mV = (float)analogRead(pinT1) * (3300.0 / 1024.0);  //Temperature measurement (T1) and gain compensation (V)
  float degC = (mV - 500.0) / 10.0;                         //conversion from volts to ºC from the previous T1 measurement

  //*******************************************************************************************
  //Introduce your control algorithm here. degC is the measurement, and u is the control output
  //*******************************************************************************************
  //u = 0.3;                   //by default the control action (u) is set to its maximum power 1 W, in future cases you should modify this!!!
  u_ant = u;
  err_ant = error;
  error = setpoint - degC;
  u = u_ant + (0.0281 * error) - (0.028 * err_ant);

  u_dig = u * 255;                  //conversion from Watts to digital (255 is the maximum value of an analogic output)
  u_dig = max(0, min(255, u_dig));  //satuartion of the analog output (upper bound: 255, lower bound: 0)
  analogWrite(pinQ1, u_dig);        //The PWM output is loaded with the digital value of u

  Serial.println(degC);  //The temperature value of T1 is transmitted by serial communication
  //************************************************************************************************
  //You can add serial communication lines to sniffer other system parameters for debugging (e.g., u-> Serial.println(degC))
  //************************************************************************************************


  // A red LED indicator lights up when the temperature exceeds 50ºC
  if (degC < 40.0) {
    analogWrite(pinLED, 0);
  } else {
    analogWrite(pinLED, 255);
  }

  // Overheating protection, if the temperature is above 80 ºC the system turns off the heater
  if (degC >= 80.0) {
    Q1 = 0.0;
    analogWrite(pinQ1, 0);
    Serial.println("High Temp 1 (>80C): ");
    Serial.println(degC);
  }
}

void loop() {
  //The control is done in the interruption loop
}
