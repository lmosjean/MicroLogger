int sensorValue = 0;        // value read from the pot
int outputValue = 0;        // value output to the PWM (analog out)
int sensorValue2 = 0;        // value read from the pot
int outputValue2 = 0;   

int Ro = 100, B =  3950; 
int Rseries = 8;
float To = 298.15;
float TREAD;

double temp = 0;

bool configMode = false;
#include <ThreeWire.h> //INCLUSÃO DA BIBLIOTECA
#include <RtcDS1302.h> //INCLUSÃO DA BIBLIOTECA

#include <SD.h>

const int chipSelect = 4;  // CS pin for SD card module

File dataFile;

ThreeWire myWire(4, 5, 2); //OBJETO DO TIPO ThreeWire
RtcDS1302<ThreeWire> Rtc(myWire); //OBJETO DO TIPO RtcDS1302

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
  while (!Serial) {
        delay(1000);
    }
    Serial.begin(9600); //INICIALIZA A SERIAL
    Rtc.Begin(); //INICIALIZA O RTC
    Serial.print("Compilado em: "); //IMPRIME O TEXTO NO MONITOR SERIAL
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__); //VARIÁVEL RECEBE DATA E HORA DE COMPILAÇÃO
    printDateTime(compiled); //PASSA OS PARÂMETROS PARA A FUNÇÃO printDateTime
    Serial.println(); //QUEBRA DE LINHA NA SERIAL
    Serial.println(); //QUEBRA DE LINHA NA SERIAL

    if(Rtc.GetIsWriteProtected()){ //SE O RTC ESTIVER PROTEGIDO CONTRA GRAVAÇÃO, FAZ
        Serial.println("RTC está protegido contra gravação. Habilitando a gravação agora..."); //IMPRIME O TEXTO NO MONITOR SERIAL
        Rtc.SetIsWriteProtected(false); //HABILITA GRAVAÇÃO NO RTC
        Serial.println(); //QUEBRA DE LINHA NA SERIAL
    }

    if(!Rtc.GetIsRunning()){ //SE RTC NÃO ESTIVER SENDO EXECUTADO, FAZ
        Serial.println("RTC não está funcionando de forma contínua. Iniciando agora..."); //IMPRIME O TEXTO NO MONITOR SERIAL
        Rtc.SetIsRunning(true); //INICIALIZA O RTC
        Serial.println(); //QUEBRA DE LINHA NA SERIAL
    }

    RtcDateTime now = Rtc.GetDateTime(); //VARIÁVEL RECEBE INFORMAÇÕES
    if (now < compiled) { //SE A INFORMAÇÃO REGISTRADA FOR MENOR QUE A INFORMAÇÃO COMPILADA, FAZ
        Serial.println("As informações atuais do RTC estão desatualizadas. Atualizando informações..."); //IMPRIME O TEXTO NO MONITOR SERIAL
        Rtc.SetDateTime(compiled); //INFORMAÇÕES COMPILADAS SUBSTITUEM AS INFORMAÇÕES ANTERIORES
        Serial.println(); //QUEBRA DE LINHA NA SERIAL
    }
    else if (now > compiled){ //SENÃO, SE A INFORMAÇÃO REGISTRADA FOR MAIOR QUE A INFORMAÇÃO COMPILADA, FAZ
        Serial.println("As informações atuais do RTC são mais recentes que as de compilação. Isso é o esperado."); //IMPRIME O TEXTO NO MONITOR SERIAL
        Serial.println(); //QUEBRA DE LINHA NA SERIAL
    }
    else if (now == compiled) { //SENÃO, SE A INFORMAÇÃO REGISTRADA FOR IGUAL A INFORMAÇÃO COMPILADA, FAZ
        Serial.println("As informações atuais do RTC são iguais as de compilação! Não é o esperado, mas está tudo OK."); //IMPRIME O TEXTO NO MONITOR SERIAL
        Serial.println(); //QUEBRA DE LINHA NA SERIAL
    }
    // Print initial message
    Serial.println("Type 'config' to enter configuration mode.");
    if (!SD.begin(chipSelect)) {
      Serial.println(F("Card failed, or not present"));
      return;
    }
    Serial.println(F("Card initialized."));
  
    // Open a file named "data.csv"
    dataFile = SD.open("data.csv", FILE_WRITE);
  
    if (dataFile) {
      Serial.println(F("File opened successfully."));
      dataFile.println("Temperature,Moisture");  // Write header to the file
      //dataFile.close();
    } else {
      Serial.println(F("Error opening file."));
    }
    // Enter the loop
    loop();
}

#define countof(a) (sizeof(a) / sizeof(a[0]))

//ACESSORES ARQUIVOS E FORMATACAO

void appendDataToFile(String data) {
  // Open the file in append mode
  dataFile = SD.open("data.csv", FILE_WRITE);

  if (dataFile) {
    dataFile.println(data);  // Append data to the file
    //dataFile.close();
    Serial.println(F("Data appended to file."));
  } else {
    Serial.println(F("Error appending data to file."));
  }
}

// AQUISICAO DE DADOS DOS SENSORES
int getMoisture(){
    int sensorValue = analogRead(A1);
    int moisturePercentage = map(sensorValue, 0, 1023, 100, 0);
    return moisturePercentage;
}

int getTemp(){
  float Vi = analogRead(A0) * (5.0 / 1023.0);
  float R = (Vi * Rseries) / (5- Vi);
  float T =  1 / ((1 / To) + ((log(R / Ro)) / B));
  TREAD= T - 273.15;
  //Serial.print(TREAD);
  return TREAD;
}

void printDateTime(const RtcDateTime& dt){
    char datestring[20]; //VARIÁVEL ARMAZENA AS INFORMAÇÕES DE DATA E HORA

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u/%02u/%04u %02u:%02u:%02u"), //FORMATO DE EXIBIÇÃO DAS INFORMAÇÕES
            dt.Day(), //DIA
            dt.Month(), //MÊS
            dt.Year(), //ANO
            dt.Hour(), //HORA
            dt.Minute(), //MINUTOS
            dt.Second() ); //SEGUNDOS
    Serial.print(datestring); //IMPRIME NO MONITOR SERIAL AS INFORMAÇÕES
}


//-------------------------------------------------------------
//CONFIGURACAO DO MODULO RTC

void setDateTimeManually() {
    Serial.println("Enter the date and time in the format 'YYYY-MM-DD HH:MM:SS'");
    Serial.println("Use a space between date and time components.");
    Serial.println("Press Enter when done.");

    while (!Serial.available()) {
        // Wait for user input
    }

    // Read user input
    String input = Serial.readStringUntil('\n');

    // Parse the input
    int year, month, day, hour, minute, second;
    if (sscanf(input.c_str(), "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second) == 6) {
        RtcDateTime newDateTime(year, month, day, hour, minute, second);

        // Update the RTC
        Rtc.SetDateTime(newDateTime);

        Serial.println("Date and time set successfully!");
        Serial.print("New date and time: ");
        printDateTime(newDateTime);
    } else {
        Serial.println("Invalid input. Please try again.");
        setDateTimeManually();  // Retry if the input is invalid
    }
}

String dataFormat = "";

//LOOP PRINCIPAL
void loop() {
   // Check for commands from Serial Monitor
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        if (command == "config") {
            Serial.println("Entering configuration mode. Type the new date and time.");
            configMode = true;
        }
        // Add more commands as needed
    }
    if (configMode) {
        // Configuration mode
        setDateTimeManually();
        configMode = false;  // Exit configuration mode after setting date and time
    } else {
        float temperatura = getTemp();
        int umidade = getMoisture();
//        
//        sensorValue = analogRead(A0);
//        outputValue = map(sensorValue, 0, 1023, 0, 255);
//        //Serial.print(temperatura);
//        //Serial.print(",");
//
//        sensorValue2 = analogRead(A1);
//        outputValue2 = map(sensorValue2, 0, 1023, 0, 255);
//        //Serial.println(umidade);

        
         dataFormat =  String(getMoisture()) + "," + String(getTemp());
         Serial.println(dataFormat);


    }

   

    // Wait before the next loop
    delay(50);
}
