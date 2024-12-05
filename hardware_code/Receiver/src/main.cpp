#include <esp_now.h>
#include <WiFi.h>

// Receiver MAC: 08:a6:f7:b1:0d:c4
// Rover 1 MAC: cc:db:a7:3e:bf:28
// Rover 2 MAX: e8:6b:ea:df:7e:f4

uint8_t Rover1Address[] = {0xCC, 0xDB, 0xA7, 0x3E, 0xBF, 0x28};
uint8_t Rover2Address[] = {0xe8, 0x6b, 0xea, 0xdf, 0x7e, 0xf4};

double tempIn;
int humIn;
float latIn;
float lonIn;
int carMove = 1;
int carId;

const unsigned long intervaloMuestreo = 1000; // Intervalo de muestreo en milisegundos (1 segundo)
unsigned long tiempoAnterior = 0;             // Variable para almacenar el último tiempo de muestreo

String message = "";

esp_now_peer_info_t peerInfo;

typedef struct struct_message
{
  int id;
  double temp;
  int hum;
  float lat;
  float lon;
  char move[2];

} struct_message;

struct_message myData;
struct_message motor_data;

struct_message car1Data;
struct_message car2Data;

struct_message carStruct[2] = {car1Data, car2Data};

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

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len)
{
  char macStr[18];
  // Serial.print("Packet received from: ");
  // snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
  //          mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  // Serial.println(macStr);
  memcpy(&myData, incomingData, sizeof(myData));
  // Serial.printf("Board ID %u: %u bytes\n", myData.id, len);
  // Update the structures with the new incoming data
  carStruct[myData.id - 1].temp = myData.temp;
  carStruct[myData.id - 1].hum = myData.hum;
  carStruct[myData.id - 1].lat = myData.lat;
  carStruct[myData.id - 1].lon = myData.lon;
  // Serial.printf("Temp value: %.2f \n", carStruct[myData.id - 1].temp);
  // Serial.printf("Hum value: %d \n", carStruct[myData.id - 1].hum);
  // Serial.printf("Lat value: %f \n", carStruct[myData.id - 1].lat);
  // Serial.printf("Lon value: %f \n", carStruct[myData.id - 1].lon);

  // Serial.println();
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
  // register peer
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  memcpy(peerInfo.peer_addr, Rover1Address, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }
  // register second peer
  memcpy(peerInfo.peer_addr, Rover2Address, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
}

void loop()
{

  if (Serial.available() > 0)
  {
    char data = Serial.read();
    // Read all text and save it in a
    //Serial.print(data);
    motor_data.move[0] = data;
  }
  unsigned long tiempoActual = millis(); // Obtiene el tiempo actual en milisegundos

  // Acess the variables for each board
  float car1temp = carStruct[0].temp;
  int car1hum = carStruct[0].hum;
  float car1lat = carStruct[0].lat;
  float car1lon = carStruct[0].lon;
  float car2temp = carStruct[1].temp;
  int car2hum = carStruct[1].hum;
  float car2lat = carStruct[1].lat;
  float car2lon = carStruct[1].lon;

  esp_err_t result = esp_now_send(Rover1Address, (uint8_t *)&motor_data, sizeof(motor_data));
  esp_err_t result1 = esp_now_send(Rover2Address, (uint8_t *)&motor_data, sizeof(motor_data));

  // Verifica si ha pasado el tiempo de muestreo
  if (tiempoActual - tiempoAnterior >= intervaloMuestreo)
  {
    tiempoAnterior = tiempoActual; // Actualiza el tiempo anterior

    message = "T1: " + String(car1temp) + ",H1: " + String(car1hum) + ",La1: " + String(car1lat, 6) + ",Lo1: " + String(car1lon, 6) + ",T2: " + String(car2temp) + ",H2:" + String(car2hum) + ",La2:" + String(car2lat, 6) + ",Lo2:" + String(car2lon, 6);

    Serial.println(message);
  }
}
