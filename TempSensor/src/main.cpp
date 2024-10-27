#include <ArduinoOTA.h>
#include <max6675.h>

#define pinage_fan 15
#define CLK_temp 11
#define CS_temp 9
#define SO_temp 13
#define pinage_res 17
#define pinage_flow_sensor 8

long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;

float calibrationFactor = 4.5;
volatile byte pulseCount;
byte pulse1Sec = 0;
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

MAX6675 thermocouple(CLK_temp, CS_temp, SO_temp);

void IRAM_ATTR pulseCounter()
{
  pulseCount++;
}

void setup() {
  Serial.begin(115200);
  ledcSetup(0,25000,8);
  ledcAttachPin(pinage_fan,0);
  pinMode(pinage_res,OUTPUT);
  digitalWrite(pinage_res,HIGH);
  
  pinMode(pinage_flow_sensor, INPUT_PULLUP);

  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  previousMillis = 0;

  attachInterrupt(digitalPinToInterrupt(pinage_flow_sensor), pulseCounter, FALLING);
}

void loop() {
  ledcWrite(0,255); 
  delay(1000);
    //temp
  Serial.print("C = "); 
  Serial.println(thermocouple.readCelsius());
  Serial.print("F = ");
  Serial.println(thermocouple.readFahrenheit());
currentMillis = millis();
  if (currentMillis - previousMillis > interval) {
    
    pulse1Sec = pulseCount;
    pulseCount = 0;

    // Because this loop may not complete in exactly 1 second intervals we calculate
    // the number of milliseconds that have passed since the last execution and use
    // that to scale the output. We also apply the calibrationFactor to scale the output
    // based on the number of pulses per second per units of measure (litres/minute in
    // this case) coming from the sensor.
    flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
    previousMillis = millis();

    // Divide the flow rate in litres/minute by 60 to determine how many litres have
    // passed through the sensor in this 1 second interval, then multiply by 1000 to
    // convert to millilitres.
    flowMilliLitres = (flowRate / 60) * 1000;

    // Add the millilitres passed in this second to the cumulative total
    totalMilliLitres += flowMilliLitres;
    
    // Print the flow rate for this second in litres / minute
    Serial.print("Flow rate: ");
    Serial.print(int(flowRate));  // Print the integer part of the variable
    Serial.print("L/min");
    Serial.print("\t");       // Print tab space

    // Print the cumulative total of litres flowed since starting
    Serial.print("Output Liquid Quantity: ");
    Serial.print(totalMilliLitres);
    Serial.print("mL / ");
    Serial.print(totalMilliLitres / 1000);
    Serial.println("L");
  }
  
}