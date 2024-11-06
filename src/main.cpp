#include <ArduinoOTA.h>

#define pinage_fan 15
#define pinage_res 17

#define PT100 5


long millisTemp = 0;


const int N = 10;

int currentPwmValue = 0;
float readings[N]; // Array para armazenar as últimas N leituras
int readIndex = 0; // Índice atual no array de leituras
int total = 0; // Soma das leituras
int average = 0; // Média das leituras

double mapToRange(int value, int in_min, int in_max, double out_min, double out_max) 
{ 
  return (double)(value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min; 
}

float smoothedTemp(){
  total = total - readings[readIndex];
  readings[readIndex] = analogRead(PT100);
  total = total + readings[readIndex];
  readIndex = readIndex + 1;

  if (readIndex >= N) {
    readIndex = 0;
  }
  average = total / N;
  return average;
}

double getTemperature(){
  float average = smoothedTemp();
  double temp = mapToRange(average, 0, 1023, 0.0, 100.0);

  return (0.91 * temp) + (8.35);

}



void setFanSpeed(int speed){
  currentPwmValue = map(speed, 0, 100, 0, 255);
  ledcWrite(0,currentPwmValue);
}


void setResistance(int resistance){
  digitalWrite(pinage_res,resistance == 0 ? LOW : HIGH);
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
  Serial.print("Resistência: ");
  Serial.println(resistaneStatus == 0 ? "Desligada" : "Ligada");
  Serial.print("Fan Speed Initial: ");
  Serial.println(map(currentPwmValue, 0, 255, 0, 100) + "%");
  Serial.println("Setup Finalizado");
}


void setup() {
  Serial.begin(115200);
  delay(5000);
  ledcSetup(0,25000,8);
  ledcAttachPin(pinage_fan,0);
  pinMode(pinage_res,OUTPUT);
  pinMode(PT100, INPUT);
  analogReadResolution(10);
  for (int i = 0; i < N; i++) {
    readings[i] = 0;
  }

  setFanSpeed(100);
  setResistance(1);
  printSetup();
  
}



void loop() {

  decisionFan();
  Serial.print("Temperatura: ");
  Serial.println(getTemperature()); 
  Serial.print("Fan Speed: ");
  Serial.print(map(currentPwmValue,0,255,0,100));
  Serial.println("%");

  delay(500);
}