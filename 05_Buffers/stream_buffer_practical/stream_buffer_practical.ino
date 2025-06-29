/**
 * ESP32 Practical Stream Buffer Example
 * 
 * Uses capacitive touch pin to stream touch intensity values via Stream Buffer.
 * Receiver task toggles LED based on intensity and logs to Serial.
 * 
 * Date: June 23, 2025
 * Author: XAI Assistant
 * License: 0BSD
 */

#include <Arduino.h>
#include <stream_buffer.h>

// Globals
static StreamBufferHandle_t touchStreamBuffer = NULL;

// Buffer size and trigger level
static const int STREAM_BUFFER_SIZE = 256;
static const int TRIGGER_LEVEL = 4; // Minimum bytes to unblock receiver

// Pins
static const int led_pin = 2;       // Built-in LED (GPIO 2 on most ESP32 boards)
static const int touch_pin = T0;    // Capacitive touch pin (GPIO 4)

// Touch thresholds
static const int TOUCH_THRESHOLD = 40;     // For detecting touch
static const int LED_TOUCH_THRESHOLD = 30; // For LED toggle

// Use only core 1 for demo purposes if unicore, otherwise multi-core is utilized
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

//*****************************************************************************
// Tasks

// Task: Streams touch intensity values during touch events
void touchSenderTask(void *pvParameters) {
  uint16_t touchValue;           // Current touch sensor reading
  unsigned long touchStartTime;  // Timestamp of touch start
  bool isTouching = false;       // Track touch state

  while (1) {
    touchValue = touchRead(touch_pin); // Read the touch sensor
    if (touchValue < TOUCH_THRESHOLD && !isTouching) { // Touch started
      isTouching = true;
      touchStartTime = millis();
      Serial.printf("Sender [Core %d]: Touch started\n", xPortGetCoreID());
    } else if (touchValue >= TOUCH_THRESHOLD && isTouching) { // Touch ended
      isTouching = false;
      unsigned long duration = millis() - touchStartTime;
      Serial.printf("Sender [Core %d]: Touch ended, duration: %lu ms\n", xPortGetCoreID(), duration);
    }
    if (isTouching) { // Stream only during touch
      // Send 16-bit touch value to the stream buffer
      size_t bytesSent = xStreamBufferSend(
        touchStreamBuffer,
        &touchValue,
        sizeof(touchValue),
        pdMS_TO_TICKS(10) // 10ms timeout
      );
      if (bytesSent == sizeof(touchValue)) {
        // Log every 100ms to avoid flooding Serial
        static unsigned long lastLog = 0;
        if (millis() - lastLog > 100) {
          Serial.printf("Sender [Core %d]: Streaming value: %u\n", xPortGetCoreID(), touchValue);
          lastLog = millis();
        }
      } else {
        Serial.printf("Sender [Core %d]: Failed to send value\n", xPortGetCoreID());
      }
    }
    vTaskDelay(pdMS_TO_TICKS(20)); // Check touch sensor every 20ms
  }
}

// Task: Receives streamed values and controls LED based on intensity
void receiverTask(void *pvParameters) {
  uint16_t touchValue; // Buffer to hold received data
  bool ledState = false; // Current LED state

  while (1) {
    // Wait indefinitely for data from the stream buffer
    size_t bytesReceived = xStreamBufferReceive(
      touchStreamBuffer,
      &touchValue,
      sizeof(touchValue),
      portMAX_DELAY
    );
    if (bytesReceived == sizeof(touchValue)) {
      // Control LED based on touch intensity
      bool newLedState = (touchValue < LED_TOUCH_THRESHOLD);
      if (newLedState != ledState) {
        ledState = newLedState;
        digitalWrite(led_pin, ledState ? HIGH : LOW);
        Serial.printf("Receiver [Core %d]: LED %s (Value: %u)\n", 
                      xPortGetCoreID(), ledState ? "ON" : "OFF", touchValue);
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
  Serial.println("\n---ESP32 Practical Stream Buffer Example---");

  // Configure the LED pin
  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, LOW);

  // Create the stream buffer
  touchStreamBuffer = xStreamBufferCreate(STREAM_BUFFER_SIZE, TRIGGER_LEVEL);
  if (touchStreamBuffer == NULL) {
    Serial.println("Error: Failed to create stream buffer");
    while (1); // Halt if buffer creation fails
  }
  Serial.println("Stream buffer created successfully");

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