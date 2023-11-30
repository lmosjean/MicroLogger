#include "SD.h"
#include"SPI.h"
#include <Wire.h> 
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//configuração do display
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define OLED_RESET     4
#define SCREEN_ADDRESS 0x3C 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//configuração do modulo sd 
#define CSpin  10
String dataString =""; // holds the data to be written to the SD card
float sensorReading1 = 0.00; // value read from your first sensor
float sensorReading2 = 0.00; // value read from your second sensor
float sensorReading3 = 0.00; // value read from your third sensor
File sensorData;

//configuração do sensor ntc
#define NTCPin  A2
#define SERIESRESISTOR 10000
#define NOMINAL_RESISTANCE 10000
#define NOMINAL_TEMPERATURE 25
#define BCOEFFICIENT 3950

//configuração do sensor de umidade
#define sensorPin  A1

//definição de variaveis para temporização
unsigned long lastReadingTime = 0; 
const unsigned long interval = 1000;

float TREAD;
int RESTO = 0;
int Ro = 100, B =  3950; 
int Rseries = 8;
float To = 298.15;
unsigned long timer =0;
unsigned long lastup=0;

void setup(){
    Serial.begin(9600);
    Serial.print("Initializing SD card...");


    pinMode(CSpin, OUTPUT);
    
    if (!SD.begin(CSpin)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
    }
    Serial.println("card initialized.");
      if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
      Serial.println(F("SSD1306 allocation failed"));
      for(;;); 
    }
    
    display.clearDisplay();
    display.setTextSize(1);             
    display.setTextColor(SSD1306_WHITE);        
    display.setCursor(0,0);             
    display.print("INICIANDO...");
    display.display();
    delay(500);
}


void loop(){
    if (millis() - lastReadingTime >= interval) {
        sensorReading1 += 0.00; // value read from your first sensor
        sensorReading2 += 0.00; // value read from your second sensor
        sensorReading3 += 0.00;
        getTemp();
        //getMoisture();
        atualizadisplay();
        lastReadingTime = millis();
  }

}


int getMoisture(){
    int sensorValue = analogRead(sensorPin);
    int moisturePercentage = map(sensorValue, 0, 1023, 100, 0);
    Serial.print("Raw Value: ");
    Serial.print(sensorValue);
    Serial.print("\t Moisture Percentage: ");
    Serial.println(moisturePercentage);
    return moisturePercentage;
}

float getTemp(){
    float Vi = analogRead(A2) * (5.0 / 1023.0);
    float R = (Vi * Rseries) / (5- Vi);
    float T =  1 / ((1 / To) + ((log(R / Ro)) / B));
    TREAD= T - 273.15;
    //if(TREAD>35 && TREAD<36){}   
    
    int temp = TREAD;
    Serial.println(temp);
    return temp;
}

void atualizadisplay(){
  if((millis()-lastup)>150){
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0,0);           
    //display.print(F("TEMP:"));
    //display.println(TSET);
    //display.setTextSize(1);            
    display.setTextColor(SSD1306_WHITE);       
    display.print(F("TEMPERATURA ATUAL:"));
    display.setCursor(0,20);                 
    //display.setCursor(0, 30);   
    display.setTextSize(4);             
    float TDISP = TREAD;
    display.println(TDISP);
    display.display();
    lastup=millis();
  } 
}

void configTime(){
    //adicionar logica de configuração da data e hora no modulo rtc
}
void getTime(){
    //adicionar logica de aquisição de data e hora
}

void saveData(){
    unsigned long currentMillis = millis();

    // Calculate date and time components
    unsigned long seconds = currentMillis / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    unsigned long days = hours / 24;

    // Extract individual components
    int year = 2000; // Set a default year
    int month, day, hour, minute;

    // Calculate date and time components
    minute = minutes % 60;
    hour = hours % 24;
    day = days % 31 + 1; // A simple placeholder for day
    month = (days / 31) % 12 + 1; // A simple placeholder for month

    // Build the formatted timestamp string
    String formattedTimestamp = String(day) + '/' + String(month) + '/' + String(year) +
                                ", " + String(hour) + ':' + String(minute);

    // Build the data string
    dataString = formattedTimestamp + ", " + String(getMoisture()) + ", " + String(getTemp());

    if(SD.exists("data.csv")){ // check if the card is still there
        // now append the new data to the file
        sensorData = SD.open("data.csv", FILE_WRITE);
        if (sensorData){
            sensorData.println(dataString);
            sensorData.close(); // close the file
        }
    }
    else{
        Serial.println("Error writing to file !");
    }
}
