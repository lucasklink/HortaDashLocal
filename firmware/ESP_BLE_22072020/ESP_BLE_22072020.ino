/* 
 *  Programa baseado no programa original desenvolvido por Timothy Woo 
 *  Tutorial do projeto original; https://www.hackster.io/botletics/esp32-ble-android-arduino-ide-awesome-81c67d
 *  Modificado para ler dados do sensor DHT11 - Bluetooth Low Energy com ESP32
 */ 
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <iostream>
#include <string>


#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme;

// GPIO where the soil_temp is connected to
const int oneWireBus = 19;
int buff=0;
float amb_humidity=0.0;
float amb_temp=0.0;
float amb_pressure=0.0;
float soil_moisture=0.0;
//float soil_temp=0.0;
float light=0.0;



// LDR is connected to GPIO 34 (Analog ADC1_CH6) 
const int LDRPin = 34;

// Soil Moisture Sensor is connected to GPIO 35 (Analog ADC1_CH7)
const int SoilMoisturePin = 35;

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature soil_temp(&oneWire);


BLECharacteristic *pCharacteristic;
 
bool deviceConnected = false;
const int LED = 2; // Could be different depending on the dev board. I used the DOIT ESP32 dev board.
 

// Veja o link seguinte se quiser gerar seus próprios UUIDs:
// https://www.uuidgenerator.net/
 
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define DHTDATA_CHAR_UUID "6E400003-B5A3-F393-E0A9-E50E24DCCA9E" 
 
 
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };
 
    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};
 
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();
      Serial.println(rxValue[0]);
 
      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");
 
        for (int i = 0; i < rxValue.length(); i++) {
          Serial.print(rxValue[i]);
        }
        Serial.println();
        Serial.println("*********");
      }
 
      // Processa o caracter recebido do aplicativo. Se for A acende o LED. B apaga o LED
      if (rxValue.find("A") != -1) { 
        Serial.println("Turning ON!");
        digitalWrite(LED, HIGH);
      }
      else if (rxValue.find("B") != -1) {
        Serial.println("Turning OFF!");
        digitalWrite(LED, LOW);
      }
    }
};
 
void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando Circuito...");
  delay(200);
  pinMode(LED, OUTPUT);
  delay(200);
  Serial.println("Iniciando Sensor de umidade do solo...");
  delay(200);
  soil_temp.begin();
  delay(200);
  Serial.println("Iniciando Sensor ambiente...");
  delay(200);
  // Start BME280 sensor
  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    //while (1);
  }
  delay(200);
  Serial.println("Iniciando Bluetooth Low Energy...");
  delay(1000);
 
  
 
  // Create the BLE Device
  BLEDevice::init("ESP32-Klink"); // Give it a name
 
  // Configura o dispositivo como Servidor BLE
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
 
  // Cria o servico UART
  BLEService *pService = pServer->createService(SERVICE_UUID);
 
  // Cria uma Característica BLE para envio dos dados
  pCharacteristic = pService->createCharacteristic(
                      DHTDATA_CHAR_UUID,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
                       
  pCharacteristic->addDescriptor(new BLE2902());
 
  // cria uma característica BLE para recebimento dos dados
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_RX,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
 
  pCharacteristic->setCallbacks(new MyCallbacks());
 
  delay(200);
  // Inicia o serviço
  pService->start();
 
  // Inicia a descoberta do ESP32
  pServer->getAdvertising()->start();
  Serial.println("Esperando um cliente se conectar...");
}
 
void loop() {
  //if (deviceConnected) {
  //
    
    soil_temp.requestTemperatures();
    float soil_temp_C = soil_temp.getTempCByIndex(0);
    //float soil_temp_F = soil_temp.getTempFByIndex(0);
    Serial.println("Temperatura do soil_temp");
    Serial.print(soil_temp_C);
    Serial.println("ºC");
    //Serial.print(soil_temp_F);
    //Serial.println("ºF");
    
    buff=analogRead(LDRPin);
    light=float(buff)/4096*100;
    Serial.print("Leitura do LDR: ");
    Serial.println(light);
    
    
    buff=analogRead(SoilMoisturePin);
    soil_moisture=float(buff)/4096*100;
    Serial.print("Leitura Soil Moisture: ");
    Serial.println(soil_moisture);

    
    amb_temp=bme.readTemperature();
    Serial.print("Leitura temp. ambiente");
    Serial.print(amb_temp);
    Serial.println("*C");

    amb_pressure=bme.readPressure() / 100.0F;
    Serial.print("Pressure = ");
    Serial.print(amb_pressure);
    Serial.println("hPa");
  
    //Serial.print("Approx. Altitude = ");
    //Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    //Serial.println("m");

    amb_humidity=bme.readHumidity();  
    Serial.print("Humidity = ");
    Serial.print(amb_humidity);
    Serial.println("%");
  
    Serial.println();
    
    char soil_temp_char[5];
    char light_char[5];
    char soil_moisture_char[5];
    char amb_temp_char[5];
    char amb_pressure_char[6];
    char amb_humidity_char[5];
    char DataString[31];

    itoa(int(soil_temp_C*100),soil_temp_char,10);
    itoa(int(light*100),light_char,10);
    itoa(int(soil_moisture*100),soil_moisture_char,10);
    itoa(int(amb_temp*100),amb_temp_char,10);
    itoa(int(amb_pressure*100),amb_pressure_char,10);
    itoa(int(amb_humidity*100),amb_humidity_char,10);

    Serial.println(soil_temp_char);
    Serial.println(light_char);
    Serial.println(soil_moisture_char);
    Serial.println(amb_temp_char);
    Serial.println(amb_pressure_char);
    Serial.println(amb_humidity_char);
    
   
    Serial.print("*** Enviando dados: ");
    strcpy(DataString,"ST");
    strcat(DataString,soil_temp_char);
    pCharacteristic->setValue(DataString);
    pCharacteristic->notify(); // Envia o valor para o aplicativo!
    Serial.print(DataString);
    Serial.println(" ***");
    //delay(300);

    
    Serial.print("*** Enviando dados: ");
    strcpy(DataString,"LI");
    strcat(DataString,light_char);
    pCharacteristic->setValue(DataString);
    pCharacteristic->notify(); // Envia o valor para o aplicativo!
    Serial.print(DataString);
    Serial.println(" ***");
    //delay(300);

    
    Serial.print("*** Enviando dados: ");
    strcpy(DataString,"SM");
    strcat(DataString,soil_moisture_char);
    pCharacteristic->setValue(DataString);
    pCharacteristic->notify(); // Envia o valor para o aplicativo!
    Serial.print(DataString);
    Serial.println(" ***");
    //delay(300);


    Serial.print("*** Enviando dados: ");
    strcpy(DataString,"AT");
    strcat(DataString,amb_temp_char);
    pCharacteristic->setValue(DataString);
    pCharacteristic->notify(); // Envia o valor para o aplicativo!
    Serial.print(DataString);
    Serial.println(" ***");
    //delay(300);



    Serial.print("*** Enviando dados: ");
    strcpy(DataString,"AP");
    strcat(DataString,amb_pressure_char);
    pCharacteristic->setValue(DataString);
    pCharacteristic->notify(); // Envia o valor para o aplicativo!
    Serial.print(DataString);
    Serial.println(" ***");
    //delay(300);


    Serial.print("*** Enviando dados: ");
    strcpy(DataString,"AH");
    strcat(DataString,amb_humidity_char);
    pCharacteristic->setValue(DataString);
    pCharacteristic->notify(); // Envia o valor para o aplicativo!
    Serial.print(DataString);
    Serial.println(" ***");
    //delay(300);
    
    
  //}
  delay(1000);
}
