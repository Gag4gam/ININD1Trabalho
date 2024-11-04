#include <ArduinoOTA.h>
#include <max6675.h>

#define pinage_fan 15
#define CLK_temp 11
#define CS_temp 9
#define SO_temp 13
#define pinage_res 17
#define pinage_flow_sensor 8
#define N 10
#define PT100 5

long currentMillis = 0;
long previousMillis = 0;
long millisTemp = 0;
int interval = 1000;
float calibrationFactor = 4.5;
volatile byte pulseCount;
byte pulse1Sec = 0;
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;
int currentPwmValue = 0;
float alpha = 0.1; // Constante do filtro (ajuste conforme necessário)
float filtered_temp;
float tempReadings[N]; // Array para armazenar as últimas N leituras
int currentIndex = 0; // Índice atual no array de leituras


MAX6675 thermocouple(CLK_temp, CS_temp, SO_temp);

void IRAM_ATTR pulseCounter()
{
  pulseCount++;
}

int getFlowRate(){
  return int(flowRate);
}

int getTotalMilliLitres(){
  return totalMilliLitres;
}

double getTemperature(){

  return thermocouple.readCelsius();
}

void calcFlowRate(){

  currentMillis = millis();
  if (currentMillis - previousMillis > interval) {
    pulse1Sec = pulseCount;
    pulseCount = 0;

    flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
    previousMillis = millis();


    flowMilliLitres = (flowRate / 60) * 1000;

    totalMilliLitres += flowMilliLitres;
  }
}

void setFanSpeed(int speed){
  currentPwmValue = map(speed, 0, 100, 0, 255);
  ledcWrite(0,currentPwmValue);
}

void setResistance(int resistance){
  digitalWrite(pinage_res,resistance == 0 ? LOW : HIGH);
}

void printFlowRate(){
  Serial.print("Vazão: ");
  Serial.print(flowRate);
  Serial.print(" L/min\t");
  Serial.print("Total: ");
  Serial.print(totalMilliLitres);
  Serial.println("mL");
}

void decisionFan(){
  int temperature = getTemperature();
  millisTemp = millis();

  if ((millisTemp / 1000) < 10) {
    setFanSpeed(100);
    return;
  }


  if (temperature >= 80 && temperature < 90){
    setFanSpeed(100);
  }
  else if (temperature >= 70 && temperature < 80 ){
    setFanSpeed(80);
  }
  else if (temperature >= 60 && temperature < 70){
    setFanSpeed(60);
  }
  else if (temperature >= 50 && temperature < 60){
    setFanSpeed(40);
  }
  else if (temperature >= 40 && temperature < 50){
    setFanSpeed(20);
  }
  else if (temperature >= 90){
    setResistance(0);
    Serial.println("Planta Desligada Por Segurança");

  }
  else{
    setFanSpeed(0);
  }
  
  
}
void printSetup(){
  int resistaneStatus = digitalRead(pinage_res);
  Serial.println("Setup Inicializado");
  Serial.print("Resistência: ");
  Serial.println(resistaneStatus == 0 ? "Desligada" : "Ligada");
  Serial.print("Fan Speed Initial: ");
  Serial.println(map(currentPwmValue, 0, 255, 0, 100) + "%");
  Serial.println("Flow Sensor Initializado");
  Serial.println("Thermocouple Initializado");
  Serial.println("Setup Finalizado");
}


void setup() {
  Serial.begin(115200);
  delay(5000);
  ledcSetup(0,25000,8);
  ledcAttachPin(pinage_fan,0);
  pinMode(pinage_res,OUTPUT);
  pinMode(PT100, INPUT);
  pinMode(pinage_flow_sensor, INPUT_PULLUP);
  analogReadResolution(10);
  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  previousMillis = 0;
  attachInterrupt(digitalPinToInterrupt(pinage_flow_sensor), pulseCounter, FALLING);
  setFanSpeed(100);
  setResistance(0);
  printSetup();
  
}

void loop() {
  //setFanSpeed(100);
  //setResistance(0);// LIGA A RESISTÊNCIA
  //calcFlowRate();
 // decisionFan();
  //printFlowRate();
  int adcValue = analogRead(PT100);
  double temp = map(adcValue, 0, 1023, 0, 100);

  Serial.print(temp);
  Serial.print(",");
  Serial.print(getTemperature());
  Serial.print(",");
  Serial.println(adcValue);

  delay(500);
}







