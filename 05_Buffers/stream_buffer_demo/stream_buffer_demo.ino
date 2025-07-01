/**
 * ESP32 Stream Buffer Demo
 * 
 * Demonstrates streaming continuous data via a Stream Buffer.
 * Sender task streams simulated sensor data (incrementing numbers), receiver task logs them to Serial.
 * 
 * Date: June 23, 2025
 * Author: Abdullahil Mahmud Arif
 */

#include <Arduino.h>
#include <stream_buffer.h>

// Globals
static StreamBufferHandle_t streamBuffer = NULL;

// Buffer size and trigger level
static const int STREAM_BUFFER_SIZE = 256;
static const int TRIGGER_LEVEL = 4; // Minimum bytes to unblock receiver

// Use only core 1 for demo purposes if unicore, otherwise multi-core is utilized
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

//*****************************************************************************
// Tasks

// Task: Sends a stream of simulated sensor data to the stream buffer
void senderTask(void *pvParameters) {
  uint16_t sensorValue = 0; // Simulated sensor data

  while (1) {
    // Send 16-bit sensor value to the stream buffer
    size_t bytesSent = xStreamBufferSend(
      streamBuffer,
      &sensorValue,
      sizeof(sensorValue),
      pdMS_TO_TICKS(100) // 100ms timeout
    );
    if (bytesSent == sizeof(sensorValue)) {
      Serial.printf("Sender [Core %d]: Sent value: %u\n", xPortGetCoreID(), sensorValue);
      sensorValue = (sensorValue + 1) % 1000; // Increment and wrap at 1000
    } else {
      Serial.printf("Sender [Core %d]: Failed to send value\n", xPortGetCoreID());
    }
    vTaskDelay(pdMS_TO_TICKS(100)); // Send every 100ms
  }
}

// Task: Receives data from the stream buffer and logs it to Serial
void receiverTask(void *pvParameters) {
  uint16_t receivedValue; // Buffer to hold received data

  while (1) {
    // Wait indefinitely for data from the stream buffer
    size_t bytesReceived = xStreamBufferReceive(
      streamBuffer,
      &receivedValue,
      sizeof(receivedValue),
      portMAX_DELAY
    );
    if (bytesReceived == sizeof(receivedValue)) {
      Serial.printf("Receiver [Core %d]: Received value: %u\n", xPortGetCoreID(), receivedValue);
    }
  }
}

//*****************************************************************************
// Main

void setup() {
  // Initialize Serial communication
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS); // Wait for Serial to stabilize
  Serial.println("\n---ESP32 Stream Buffer Demo---");

  // Create the stream buffer
  streamBuffer = xStreamBufferCreate(STREAM_BUFFER_SIZE, TRIGGER_LEVEL);
  if (streamBuffer == NULL) {
    Serial.println("Error: Failed to create stream buffer");
    while (1); // Halt if buffer creation fails
  }
  Serial.println("Stream buffer created successfully");

  // Create the sender task on core 0
  xTaskCreatePinnedToCore(
    senderTask,    // Task function
    "Sender",      // Task name
    2048,          // Stack size in bytes
    NULL,          // Parameters
    1,             // Priority
    NULL,          // Task handle
    0              // Run on core 0
  );

  // Create the receiver task on core 1
  xTaskCreatePinnedToCore(
    receiverTask,  // Task function
    "Receiver",    // Task name
    2048,          // Stack size in bytes
    NULL,          // Parameters
    1,             // Priority
    NULL,          // Task handle
    1              // Run on core 1
  );

  Serial.println("Tasks created successfully");
}

void loop() {
  // Do nothing but yield to other tasks
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}
