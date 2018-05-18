//Incluimos bibliotecas para utilizar el RC522
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN  D9    //Pin 9 para el reset del RC522
#define SS_PIN  D10   //Pin 10 para el SS (SDA) del RC522
MFRC522 Lector1(SS_PIN, RST_PIN); ///Creamos el objeto para el RC522 al cual llamamos Lector1

const char* ssid = "AMCOM_3";
const char* password = "4MC0MS4_2017";

const char* host = "192.168.0.10";
const int httpsPort = 8000;

const int id = 1;

void setup() {
  Serial.begin(115200); //Iniciamos La comunicacion serial
  delay(250);
  //Start WiFi
  Serial.println(F("Booting...."));
  
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println(F("Ready!"));
  Serial.println(F("======================================================")); 
  //End WiFi
  
  SPI.begin();        //Iniciamos el Bus SPI
  Lector1.PCD_Init(); // Iniciamos el MFRC522
  Serial.println("Bienvenido al Sistema Lector de Tarjetas RFID\n    Por Favor Acerca tu Tarjeta al Lector\n");
}


void loop() {
      
 
  if ( Lector1.PICC_IsNewCardPresent())   // Revisamos si hay nuevas tarjetas  presentes.
  { 
    if ( Lector1.PICC_ReadCardSerial())  //Leemos la tarjeta presente.
    {
      WiFiClient client;
    
      if (!client.connect(host, httpsPort)) {
        Serial.println("connection failed");
        return;
      }
      
      unsigned long UID_unsigned;
      UID_unsigned =  Lector1.uid.uidByte[0] << 24;
      UID_unsigned += Lector1.uid.uidByte[1] << 16;
      UID_unsigned += Lector1.uid.uidByte[2] <<  8;
      UID_unsigned += Lector1.uid.uidByte[3];
    
      Serial.println();
      Serial.println("UID Unsigned int"); 
      Serial.println(UID_unsigned);
    
      String UID_string =  (String)UID_unsigned;
      
      String url = "/recibirDatos";
      Serial.print("requesting URL: ");
      Serial.println(url);

      String data = "tarjeta="+UID_string+"&id="+id;

        client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Accept: *" + "/" + "*\r\n" +
               "Content-Length: " + data.length() + "\r\n" +
               "Content-Type: application/x-www-form-urlencoded\r\n" +
               "User-Agent: Arduino/1.0\r\n" +
               "Connection: close\r\n"+
               "\r\n" + data+"\r\n\r\n");

        Serial.println("request sent");
        while (client.connected()) {
          String line = client.readStringUntil('\n');
          if (line == "\r") {
            Serial.println("headers received");
            break;  
          }
        }
        
        String line = client.readStringUntil('\n');
      
        Serial.println("reply was:");
        Serial.println("==========");
        Serial.println(line);
        Serial.println("==========");
        Serial.println("closing connection");
      
      Lector1.PICC_HaltA(); // Terminamos la lectura de la tarjeta tarjeta para ahorrar energía.
      Lector1.PCD_StopCrypto1(); //Detenemos la encriptacion.
    }
  }
}
