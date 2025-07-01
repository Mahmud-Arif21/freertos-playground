/**
 * ESP32 Practical Message Buffer Example
 * 
 * Uses capacitive touch pin to detect touch events and sends duration via Message Buffer.
 * Receiver task blinks LED based on touch duration and logs to Serial.
 * 
 * Date: June 23, 2025
 * Author: Abdullahil Mahmud Arif
 */

#include <Arduino.h>
#include <message_buffer.h>

// Globals
static MessageBufferHandle_t touchBuffer = NULL;

// Buffer size (must accommodate the largest expected message)
static const int MESSAGE_BUFFER_SIZE = 128;

// Pins
static const int led_pin = 2;       // Built-in LED (GPIO 2 on most ESP32 boards)
static const int touch_pin = T0;    // Capacitive touch pin (GPIO 4)

// Touch threshold (adjust based on board sensitivity)
static const int TOUCH_THRESHOLD = 40;

// Use only core 1 for demo purposes if unicore, otherwise multi-core is utilized
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

//*****************************************************************************
// Tasks

// Task: Monitors touch sensor and sends duration messages
void touchSenderTask(void *pvParameters) {
  char message[64];              // Buffer for constructing the message
  int touchValue;                // Current touch sensor reading
  unsigned long touchStartTime;  // Timestamp of touch start

  while (1) {
    touchValue = touchRead(touch_pin); // Read the touch sensor
    if (touchValue < TOUCH_THRESHOLD) { // Touch detected
      touchStartTime = millis();        // Record when the touch began
      while (touchValue < TOUCH_THRESHOLD) { // Wait until touch ends
        touchValue = touchRead(touch_pin);
        vTaskDelay(pdMS_TO_TICKS(10));  // Check every 10ms
      }
      // Calculate how long the touch lasted
      unsigned long duration = millis() - touchStartTime;
      // Format the message with the duration
      snprintf(message, sizeof(message), "Touch duration: %lu ms", duration);
      // Send the message to the buffer
      size_t bytesSent = xMessageBufferSend(
        touchBuffer,
        message,
        strlen(message),
        pdMS_TO_TICKS(100) // 100ms timeout
      );
      if (bytesSent == strlen(message)) {
        Serial.printf("Sender [Core %d]: Sent: %s\n", xPortGetCoreID(), message);
      } else {
        Serial.printf("Sender [Core %d]: Failed to send message\n", xPortGetCoreID());
      }
    }
    vTaskDelay(pdMS_TO_TICKS(50)); // Check touch sensor every 50ms
  }
}

// Task: Receives messages and blinks LED based on duration
void receiverTask(void *pvParameters) {
  char buffer[64];  // Buffer to hold received messages

  while (1) {
    // Wait indefinitely for a message
    size_t bytesReceived = xMessageBufferReceive(
      touchBuffer,
      buffer,
      sizeof(buffer) - 1, // Leave space for null terminator
      portMAX_DELAY
    );
    if (bytesReceived > 0) {
      buffer[bytesReceived] = '\0'; // Null terminate
      Serial.printf("Receiver [Core %d]: Received: %s\n", xPortGetCoreID(), buffer);
      // Extract duration to determine blink count
      unsigned long duration;
      sscanf(buffer, "Touch duration: %lu ms", &duration);
      int blinks = duration / 100; // 1 blink per 100ms, capped at 5
      for (int i = 0; i < blinks && i < 5; i++) {
        digitalWrite(led_pin, HIGH);
        vTaskDelay(pdMS_TO_TICKS(200));
        digitalWrite(led_pin, LOW);
        vTaskDelay(pdMS_TO_TICKS(200));
      }
    }
  }
}

//*****************************************************************************
// Main

void setup() {
  // Initialize Serial communication
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS); // Wait for Serial to stabilize
  Serial.println("\n---ESP32 Practical Message Buffer Example---");

  // Configure the LED pin
  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, LOW);

  // Create the message buffer
  touchBuffer = xMessageBufferCreate(MESSAGE_BUFFER_SIZE);
  if (touchBuffer == NULL) {
    Serial.println("Error: Failed to create message buffer");
    while (1); // Halt if buffer creation fails
  }
  Serial.println("Message buffer created successfully");

  // Create the sender task on core 0
  xTaskCreatePinnedToCore(
    touchSenderTask,    // Task function
    "Touch Sender",     // Task name
    2048,               // Stack size in bytes
    NULL,               // Parameters
    1,                  // Priority
    NULL,               // Task handle
    0                   // Run on core 0
  );

  // Create the receiver task on core 1
  xTaskCreatePinnedToCore(
    receiverTask,       // Task function
    "Receiver",         // Task name
    2048,               // Stack size in bytes
    NULL,               // Parameters
    1,                  // Priority
    NULL,               // Task handle
    1                   // Run on core 1
  );

  Serial.println("Tasks created successfully");
}

void loop() {
  // Do nothing but yield to other tasks
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}
