/**
 * ESP32 Message Buffer Demo
 * 
 * Demonstrates sending variable-length messages via a Message Buffer.
 * Sender task sends messages of different lengths, receiver task logs them to Serial.
 * 
 * Date: June 23, 2025
 * Author: XAI Assistant
 * License: 0BSD
 */

#include <Arduino.h>
#include <message_buffer.h>

// Globals
static MessageBufferHandle_t msgBuffer = NULL;

// Buffer size (must accommodate the largest expected message)
static const int MESSAGE_BUFFER_SIZE = 128;

// Use only core 1 for demo purposes if unicore, otherwise multi-core is utilized
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

//*****************************************************************************
// Tasks

// Task: Sends messages of varying lengths to the message buffer
void senderTask(void *pvParameters) {
  const char* messages[] = {
    "Short",
    "Medium length",
    "This is a longer message to show variable-length capability",
    "Tiny",
    "Another example message"
  };
  int numMessages = 5;
  int index = 0;

  while (1) {
    const char* message = messages[index];
    size_t messageLen = strlen(message);
    // Send the message to the buffer
    size_t bytesSent = xMessageBufferSend(
      msgBuffer,
      message,
      messageLen,
      pdMS_TO_TICKS(100) // 100ms timeout
    );
    if (bytesSent == messageLen) {
      Serial.printf("Sender [Core %d]: Sent: %s\n", xPortGetCoreID(), message);
    } else {
      Serial.printf("Sender [Core %d]: Failed to send message\n", xPortGetCoreID());
    }
    index = (index + 1) % numMessages; // Cycle through messages
    vTaskDelay(pdMS_TO_TICKS(1000));   // Send every second
  }
}

// Task: Receives messages from the buffer and logs them to Serial
void receiverTask(void *pvParameters) {
  char buffer[64];  // Buffer to hold received messages

  while (1) {
    // Wait indefinitely for a message
    size_t bytesReceived = xMessageBufferReceive(
      msgBuffer,
      buffer,
      sizeof(buffer) - 1, // Leave space for null terminator
      portMAX_DELAY
    );
    if (bytesReceived > 0) {
      buffer[bytesReceived] = '\0'; // Null terminate
      Serial.printf("Receiver [Core %d]: Received: %s\n", xPortGetCoreID(), buffer);
    }
  }
}

//*****************************************************************************
// Main

void setup() {
  // Initialize Serial communication
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS); // Wait for Serial to stabilize
  Serial.println("\n---ESP32 Message Buffer Demo---");

  // Create the message buffer
  msgBuffer = xMessageBufferCreate(MESSAGE_BUFFER_SIZE);
  if (msgBuffer == NULL) {
    Serial.println("Error: Failed to create message buffer");
    while (1); // Halt if buffer creation fails
  }
  Serial.println("Message buffer created successfully");

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