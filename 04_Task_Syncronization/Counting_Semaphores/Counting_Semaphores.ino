#include <Arduino.h>

// Onboard LED  
#define LED_PIN 2

// Semaphore handles  
SemaphoreHandle_t poolSem = NULL;
SemaphoreHandle_t serialMutex = NULL;

// Aligned logger
void logMessage(int id, const char* stage, const char* msg) {
  
  if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(100))) {
    // Print aligned table row: Worker | Stage     | Message
    Serial.printf("[W%-2d] | %-9s | %s\n", id, stage, msg);
    xSemaphoreGive(serialMutex);
  }
}

// Worker task
void worker(void* pvParameters) {
  int id = (int)pvParameters;

  for (;;) {
    int available = uxSemaphoreGetCount(poolSem);
    char statusMsg[50];
    snprintf(statusMsg, sizeof(statusMsg), "Waiting… Pool Available = %d\n", available);
    logMessage(id, "WAIT", statusMsg);

    // Try acquiring resource
    xSemaphoreTake(poolSem, portMAX_DELAY);
    logMessage(id, "ACQUIRE", "Slot acquired. Blinking LED twice\n");

    // Simulate work (2 LED blinks)
    for (int i = 0; i < 2; ++i) {
      digitalWrite(LED_PIN, HIGH);
      vTaskDelay(pdMS_TO_TICKS(150));
      digitalWrite(LED_PIN, LOW);
      vTaskDelay(pdMS_TO_TICKS(150));
    }

    logMessage(id, "RELEASE", "Work done. Releasing slot\n");
    xSemaphoreGive(poolSem);

    char waitMsg[40];
    snprintf(waitMsg, sizeof(waitMsg), "Sleeping for %d sec before next round\n\n", id);
    logMessage(id, "SLEEP", waitMsg);

    vTaskDelay(pdMS_TO_TICKS(1000 * id));
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Create semaphores
  poolSem = xSemaphoreCreateCounting(2, 2);
  serialMutex = xSemaphoreCreateMutex();

  if (!poolSem || !serialMutex) {
    Serial.println("Error: Failed to create semaphores");
    while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
  }

  Serial.println("\n=== Counting Semaphore Demo (2 Slots) ===");
  Serial.println("FORMAT: [Worker] | [Stage]    | [Message]");
  Serial.println("---------------------------------------------------");

  // Launch 3 worker tasks
  xTaskCreatePinnedToCore(
    worker,               // Task function 
    "Worker 1",           // Task name
    2048,                 // Stack size (bytes) 
    (void*)1,             // Task parameter (worker number) 
    1,                    // Priority (1 is low) 
    NULL,                 // Task handle 
    0                     // Run on Core 0
    );
  
  xTaskCreatePinnedToCore(
    worker, 
    "Worker 2", 
    2048, 
    (void*)2, 
    1, 
    NULL, 
    1                     // Run on Core 0
    );
  
  xTaskCreatePinnedToCore(
    worker, 
    "Worker 3", 
    2048, 
    (void*)3, 
    1, 
    NULL, 
    1                     // Run on Core 0
    );
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}
