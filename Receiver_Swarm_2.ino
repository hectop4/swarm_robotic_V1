#include <esp_now.h>
#include <WiFi.h>

// Receiver MAC: 08:a6:f7:b1:0d:c4
// Rover 1 MAC: cc:db:a7:3e:bf:28
//Rover 2 MAX: e8:6b:ea:df:7e:f4

uint8_t Rover1Address[] = {0xCC, 0xDB, 0xA7, 0x3E, 0xBF, 0x28};
//uint8_t Rover1Address[]= {0xe8,0x6b,0xea,0xdf,0x7e,0xf4}



double tempIn;
int humIn;
float latIn;
float lonIn;
int carMove=1;


const unsigned long intervaloMuestreo = 1000; // Intervalo de muestreo en milisegundos (1 segundo)
unsigned long tiempoAnterior = 0;            // Variable para almacenar el último tiempo de muestreo

String message="";

esp_now_peer_info_t peerInfo; 

typedef struct struct_message
{
  double temp;
  int hum;
  float lat;
  float lon;
  char move[2];

} struct_message;

struct_message myData;
struct_message motor_data;

String success;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  // Serial.print("\r\nLast Packet Send Status:\t");
  // Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  // if (status == 0)
  // {
  //   success = "Delivery Success :)";
  // }
  // else
  // {
  //   success = "Delivery Fail :(";
  // }
}

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
  memcpy(&myData, incomingData, sizeof(myData));
  tempIn = myData.temp;
  humIn = myData.hum;
  latIn = myData.lat;
  lonIn = myData.lon;
}



void setup()
{
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  memcpy(peerInfo.peer_addr, Rover1Address, 6);
  peerInfo.channel=0;
  peerInfo.encrypt=false;


    // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  

  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
    esp_now_register_recv_cb(esp_now_recv_cb_t (OnDataRecv));
}

void loop()
{

if(Serial.available()>0){
    char data = Serial.read();
    //Read all text and save it in a
    Serial.print(data);
    motor_data.move[0] = data;


  }


  esp_err_t result = esp_now_send(Rover1Address, (uint8_t *) &motor_data, sizeof(motor_data));


unsigned long tiempoActual = millis(); // Obtiene el tiempo actual en milisegundos
  
  // Verifica si ha pasado el tiempo de muestreo
  if (tiempoActual - tiempoAnterior >= intervaloMuestreo) {
    tiempoAnterior = tiempoActual; // Actualiza el tiempo anterior
    

  message= "T:"+ String(myData.temp)+",H:"+ String(myData.hum) + ",La:" +String(myData.lat,6)+",Lo:"+String(myData.lon,6);

  Serial.println(message);
  }

}