#include <Arduino.h>

// ======== Configuration ==========
#define LED_PIN 2

SemaphoreHandle_t ledMutex;         // Our "LED access token"

// ======== LED Blink Task =========
void blinkTask(void *pvParameters) {
  const char* taskName = (const char*) pvParameters;
  
  pinMode(LED_PIN, OUTPUT);         // Setup LED pin
  
  for (;;) {                        // Infinite loop
    // --- GET LED ACCESS ---
    xSemaphoreTake(ledMutex, portMAX_DELAY);  // Wait forever for mutex
    
    // --- CRITICAL SECTION START ---
    Serial.printf("%s: Started blinking\n", taskName);
    
    // Blink pattern (protected from interference)
    for (int i = 0; i < 2; i++) {
      digitalWrite(LED_PIN, HIGH);
      vTaskDelay(200 / portTICK_PERIOD_MS);  // LED on 200ms
      digitalWrite(LED_PIN, LOW);
      vTaskDelay(200 / portTICK_PERIOD_MS);  // LED off 200ms
    }
    // --- CRITICAL SECTION END ---
    
    xSemaphoreGive(ledMutex);       // Release mutex
    
    // Wait before next blink attempt
    vTaskDelay(1000 / portTICK_PERIOD_MS);  // 1 second pause
  }
}

// ======== Main Setup =============
void setup() {
  Serial.begin(115200);
  
  vTaskDelay(1000 / portTICK_PERIOD_MS);  // Wait for serial
  Serial.println("\nLED Mutex Demo Start");

  // Create our mutex (like making a key)
  ledMutex = xSemaphoreCreateMutex();
  
  if (ledMutex == NULL) {
    Serial.println("Failed to create mutex!");
    while(1);  // Freeze if mutex fails
  }

  // Create two competing tasks
  xTaskCreatePinnedToCore(
    blinkTask,    // Task function
    "BlinkA",     // Task name
    1024,         // Stack size
    (void*)"TaskA",  // Parameter (text)
    1,            // Priority
    NULL,         // Task handle
    0             // Core 0
  );

  xTaskCreatePinnedToCore(
    blinkTask,
    "BlinkB",
    1024,
    (void*)"TaskB",
    1,
    NULL,
    1             // Core 1
  );
}

void loop() {
  // Execution should never get here
}
