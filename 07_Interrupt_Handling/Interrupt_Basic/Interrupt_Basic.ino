/**
 * FreeRTOS Interrupt Basic Demo
 * 
 * Simulates a button press via serial input to notify a task.
 */

#include <Arduino.h>
#include <freertos/FreeRTOS.h>

// Use core 0 for demo purposes
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 0;
#endif

// Global variables
static TaskHandle_t printTaskHandle = NULL;

// Simulated ISR: Notifies task on serial input 'p'
void simulateInterrupt() {
  if (Serial.available() > 0) {
    char c = Serial.read();
    if (c == 'p') { // Press 'p' to simulate interrupt
      xTaskNotify(printTaskHandle, 0, eNoAction);
    }
  }
}

// Task: Prints message on notification
void printTask(void* p) {
  while (1) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // Wait for notification
    Serial.println("Interrupt event occurred!");
  }
}

// Main
void setup() {
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS); // Wait for serial
  Serial.println("\nBasic Interrupt Demo Starting");

  // Create task
  xTaskCreatePinnedToCore(
    printTask,    // Function
    "Print Task", // Name
    1024,         // Stack size
    NULL,         // Parameters
    1,            // Priority
    &printTaskHandle, // Handle
    app_cpu       // Core
  );

  // Delete setup task
  vTaskDelete(NULL);
}

void loop() {
  simulateInterrupt();
  vTaskDelay(pdMS_TO_TICKS(10)); // Polling interval
}