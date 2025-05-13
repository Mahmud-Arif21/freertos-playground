#include <Arduino.h>

// Define onboard LED
#define LED_PIN 2

// Task handles
TaskHandle_t triggerTaskHandle = NULL;
TaskHandle_t responderTaskHandle = NULL;

// Semaphore handle
SemaphoreHandle_t syncSemaphore = NULL;

// Trigger Task - Periodically signals the responder task
void triggerTask(void *pvParameters) {
  for(;;) {
    int core = xPortGetCoreID();
    
    // Wait for 3 seconds
    Serial.printf("Trigger [Core %d]: Waiting 3 seconds...\n", core);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    
    // Signal the responder task by giving the semaphore
    Serial.printf("Trigger [Core %d]: Signaling responder task\n", core);
    xSemaphoreGive(syncSemaphore);
  }
}

// Responder Task - Waits for signal and blinks LED
void responderTask(void *pvParameters) {
  for(;;) {
    int core = xPortGetCoreID();
    
    // Wait indefinitely for the semaphore
    Serial.printf("Responder [Core %d]: Waiting for signal...\n", core);
    if(xSemaphoreTake(syncSemaphore, portMAX_DELAY) == pdTRUE) {
      // Semaphore obtained, blink the LED
      Serial.printf("Responder [Core %d]: Signal received! Blinking LED\n", core);
      
      // Blink LED 5 times quickly
      for(int i = 0; i < 5; i++) {
        digitalWrite(LED_PIN, HIGH);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        digitalWrite(LED_PIN, LOW);
        vTaskDelay(100 / portTICK_PERIOD_MS);
      }
    }
  }
}

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  
  // Configure LED pin
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // Print startup message
  Serial.println("\n=== ESP32 FreeRTOS Binary Semaphore Example ===");
  
  // Create a binary semaphore
  syncSemaphore = xSemaphoreCreateBinary();
  
  if(syncSemaphore == NULL) {
    Serial.println("Error: Could not create semaphore");
    while(1); // Stop if semaphore creation failed
  }
  
  // Create the trigger task on Core 0
  xTaskCreatePinnedToCore(
    triggerTask,         // Task function
    "Trigger Task",      // Task name
    2048,                // Stack size (bytes)
    NULL,                // Task parameters
    1,                   // Priority (1 is low)
    &triggerTaskHandle,  // Task handle
    0                    // Run on Core 0
  );
  
  // Create the responder task on Core 1
  xTaskCreatePinnedToCore(
    responderTask,       // Task function
    "Responder Task",    // Task name
    2048,                // Stack size
    NULL,                // Task parameters
    1,                   // Priority (1 is low)
    &responderTaskHandle,// Task handle
    1                    // Run on Core 1
  );
  
  Serial.println("All tasks created!");
}

void loop() {
  // Empty loop - all the work is done in tasks
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}
