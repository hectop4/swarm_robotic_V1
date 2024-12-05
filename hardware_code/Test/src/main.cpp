#include <Arduino.h>

void setup()
{
  // Initialize serial communication at 115200 baud rate
  Serial.begin(115200);
}

void loop()
{
  // Check if data is available to read
  if (Serial.available() > 0)
  {
    // Read the incoming byte
    int incomingByte = Serial.read();

    // Print the received byte to the serial monitor
    Serial.print("Received: ");
    Serial.println(incomingByte);
  }
}