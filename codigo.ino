bool configMode = false;
#include <ThreeWire.h> //INCLUSÃO DA BIBLIOTECA
#include <RtcDS1302.h> //INCLUSÃO DA BIBLIOTECA

#include <SD.h>
File sensorData;

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
    //printDateTime(compiled); //PASSA OS PARÂMETROS PARA A FUNÇÃO printDateTime
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
    if (!SD.begin(4)) {
      Serial.println(F("Card failed, or not present"));
      return;
    }
    Serial.println(F("Card initialized."));
  
    // Open a file named "data.csv"
    sensorData = SD.open("data.csv", FILE_WRITE);
  
    if (sensorData) {
      Serial.println(F("File opened successfully."));
      sensorData.println("Temperature,Moisture");  // Write header to the file
      //sensorData.close();
    } else {
      Serial.println(F("Error opening file."));
    }
 
}

#define countof(a) (sizeof(a) / sizeof(a[0]))

//ACESSORES ARQUIVOS E FORMATACAO

void appendDataToFile(String data) {
  if(SD.exists("data.csv")){ // check if the card is still there
        if (sensorData){
          Serial.print("datafile:");
          Serial.println(data);
//            sensorData.println(data);
            Serial.println("Dados gravados no sd");
            //sensorData.close(); // close the file
        }
    }
    else{
        Serial.println("Error writing to file !");
    }
}

// AQUISICAO DE DADOS DOS SENSORES
int getMoisture(){
    int moisturePercentage = map(analogRead(A1), 0, 1023, 100, 0);
    return moisturePercentage;
}

int getTemp(){
  float Vi = analogRead(A0) * (5.0 / 1023.0);
  float R = (Vi * 8) / (5- Vi);
  float T =  1 / ((1 / 298.15) + ((log(R / 100)) / 3950));
  float TREAD= T - 273.15;
  //Serial.print(TREAD);
  return TREAD;
}
//-------------------------------------------------------------


//CONFIGURACAO DO MODULO RTC
//String getFormattedDateTime(const RtcDateTime& dt) {
//    int dia = dt.Day();
//    int mes = dt.Month();
//    int ano = dt.Year();
//    int hora = dt.Hour();
//    int minuto = dt.Minute();
//    int segundo = dt.Second();
//
//    String datahora = String(dia) + "-" + String(mes) + "-" + String(ano)  + "," +
//                      String(hora) + ":" + String(minuto) + ":" + String(segundo);
//
//    return datahora;
//}

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
        //printDateTime(newDateTime);
    } else {
        Serial.println("Invalid input. Please try again.");
        setDateTimeManually();  // Retry if the input is invalid
    }
}

//

//LOOP PRINCIPAL

long lastreading =0;

long lastreading2 =0;


void loop() {
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
     if(millis() - lastreading >= 500){
        String dataFormat = String(getMoisture()) + "," + String(getTemp());
//        Serial.println(dataFormat);
          if(millis() - lastreading2 >= 500){
             appendDataToFile(dataFormat);
             lastreading2 = millis();
             }
        lastreading = millis();
        
    }

}
}

