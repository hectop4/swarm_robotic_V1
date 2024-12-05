
#include <Wire.h>
#include <DHT.h>
#include <DHT_U.h>
#include <HardwareSerial.h>
#include <TinyGPS++.h>
#include <esp_now.h>
#include <WiFi.h>

// Rover 1 MAC: cc:db:a7:3e:bf:28
// Rover 2 MAC: e8:6b:ea:df:7e:f4
// Receiver MAC: 08:a6:f7:b1:0d:c4

#define led_pin 2

uint8_t receiverAddress[] = {0x08, 0xA6, 0xF7, 0xB1, 0x0D, 0xC4};

// Motor control pins
const int motor1Pin1 = 13;
const int motor1Pin2 = 25;
const int motor2Pin1 = 26;
const int motor2Pin2 = 27;

const unsigned long intervaloMuestreo = 1000; // Intervalo de muestreo en milisegundos (1 segundo)
unsigned long tiempoAnterior = 0;             // Variable para almacenar el último tiempo de muestreo

// GPS
HardwareSerial gpsSerial(2);
TinyGPSPlus gps;
float lat, lon;

// DHT11
#define DHTPIN 21
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

int hum;
double temp;

char carMoveIn[2];

// Communication
//  Variable to store if sending data was successful
String success;

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

esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status == 0)
  {
    success = "Delivery Success :)";
  }
  else
  {
    success = "Delivery Fail :(";
  }
}

// Callback when data is received
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
  memcpy(&motor_data, incomingData, sizeof(motor_data));
  carMoveIn[0] = motor_data.move[0];
}

void forward()
{
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, HIGH);
  digitalWrite(motor2Pin1, HIGH);
  digitalWrite(motor2Pin2, LOW);
}

void backward()
{
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, HIGH);
}

void right()
{
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, HIGH);
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, HIGH);
}
void left()
{
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor2Pin1, HIGH);
  digitalWrite(motor2Pin2, LOW);
}

void stop()
{
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, LOW);
}

void getReadings()
{
  while (gpsSerial.available() > 0)
  {
    gps.encode(gpsSerial.read());
  }

  // Print GPS data
  if (gps.location.isUpdated())
  {
    lat = gps.location.lat();
    lon = gps.location.lng();
    // Serial.print("Latitude: ");
    // Serial.println(gps.location.lat(), 6);
    // Serial.print("Longitude: ");
    // Serial.println(gps.location.lng(), 6);
  }

  // Read DHT11 data
  hum = dht.readHumidity();
  temp = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(hum) || isnan(temp))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
}

void setup()
{
  // Initialize serial communication
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  gpsSerial.begin(9600, SERIAL_8N1, 16, 17); // RX2 = GPIO16, TX2 = GPIO17

  // Initialize motor control pins
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(motor2Pin1, OUTPUT);
  pinMode(motor2Pin2, OUTPUT);
  pinMode(led_pin, OUTPUT);

  // Initialize DHT11
  dht.begin();

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  memcpy(peerInfo.peer_addr, receiverAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
  // Register for a callback function that will be called when data is received

  digitalWrite(led_pin, HIGH);
  delay(10000);
  digitalWrite(led_pin, LOW);
  delay(500);
  digitalWrite(led_pin, HIGH);
  delay(100);
  digitalWrite(led_pin, LOW);
}

void loop()
{
  myData.id = 2;
  unsigned long tiempoActual = millis(); // Obtiene el tiempo actual en milisegundos
  // Verifica si ha pasado el tiempo de muestreo
  if (tiempoActual - tiempoAnterior >= intervaloMuestreo)
  {
    tiempoAnterior = tiempoActual; // Actualiza el tiempo anterior

    // Código a ejecutar en cada intervalo de muestreo

    getReadings();

    myData.temp = temp;
    myData.hum = hum;
    myData.lat = lat;
    myData.lon = lon;
  }

  switch (carMoveIn[0])
  {
  case 'z':
    forward();
    break;
  case 'x':
    backward();
    break;
  case 'c':
    right();
    break;
  case 'v':
    left();
    break;
  case 's':
    stop();
    break;
  default:
    stop();
    break;
  }

  esp_err_t result = esp_now_send(receiverAddress, (uint8_t *)&myData, sizeof(myData));
}