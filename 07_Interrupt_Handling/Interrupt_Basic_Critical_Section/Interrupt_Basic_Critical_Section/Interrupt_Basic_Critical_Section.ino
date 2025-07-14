/**
 * FreeRTOS Interrupt Critical Section Demo
 * 
 * Demonstrates safe interrupt handling using critical sections
 * to protect shared resources between ISR and tasks.
 * 
 * Commands:
 * - 'i' - Trigger interrupt (increments counter)
 * - 'r' - Read counter value
 * - 's' - Start/stop continuous counter updates
 */

#include <Arduino.h>
#include <freertos/FreeRTOS.h>

// Use core 0 for demo purposes
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 0;
#endif

// Global shared variables (protected by critical sections)
static volatile uint32_t interruptCounter = 0;
static volatile bool continuousMode = false;
static volatile uint32_t lastReadValue = 0;

// Task handles
static TaskHandle_t counterTaskHandle = NULL;
static TaskHandle_t displayTaskHandle = NULL;

// Simulated ISR: Safely increments counter using critical section
void IRAM_ATTR simulatedISR() {
  // Enter critical section (disables interrupts)
  portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
  portENTER_CRITICAL_ISR(&mux);
  
  // Critical section: Modify shared variable
  interruptCounter++;
  
  // Exit critical section (re-enables interrupts)
  portEXIT_CRITICAL_ISR(&mux);
  
  // Notify task from ISR
  BaseType_t higherPriorityTaskWoken = pdFALSE;
  xTaskNotifyFromISR(counterTaskHandle, 0, eNoAction, &higherPriorityTaskWoken);
  portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

// Task: Handles counter events and continuous updates
void counterTask(void* p) {
  TickType_t lastWakeTime = xTaskGetTickCount();
  
  while (1) {
    // Check for interrupt notification or timeout for continuous mode
    if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(500)) > 0) {
      Serial.println("Interrupt triggered!");
    }
    
    // Handle continuous mode
    if (continuousMode) {
      // Safely read counter in critical section
      portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
      portENTER_CRITICAL(&mux);
      uint32_t currentValue = interruptCounter;
      portEXIT_CRITICAL(&mux);
      
      Serial.print("Continuous mode - Counter: ");
      Serial.println(currentValue);
      
      // Update every 1 second in continuous mode
      vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(1000));
    }
  }
}

// Task: Displays counter value when requested
void displayTask(void* p) {
  while (1) {
    // Wait for notification to display counter
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    
    // Safely read shared variables in critical section
    portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
    portENTER_CRITICAL(&mux);
    uint32_t current = interruptCounter;
    uint32_t previous = lastReadValue;
    lastReadValue = current;
    portEXIT_CRITICAL(&mux);
    
    Serial.print("Counter Value: ");
    Serial.print(current);
    Serial.print(" (Changed by: ");
    Serial.print(current - previous);
    Serial.println(")");
  }
}

// Process serial commands
void processSerialInput() {
  if (Serial.available() > 0) {
    char command = Serial.read();
    
    switch (command) {
      case 'i':
        Serial.println("Simulating interrupt...");
        simulatedISR();
        break;
        
      case 'r':
        Serial.println("Reading counter...");
        xTaskNotify(displayTaskHandle, 0, eNoAction);
        break;
        
      case 's':
        // Toggle continuous mode safely
        {
          portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
          portENTER_CRITICAL(&mux);
          continuousMode = !continuousMode;
          portEXIT_CRITICAL(&mux);
          
          Serial.print("Continuous mode: ");
          Serial.println(continuousMode ? "ON" : "OFF");
        }
        break;
        
      case 'h':
        Serial.println("\nCommands:");
        Serial.println("i - Trigger interrupt");
        Serial.println("r - Read counter");
        Serial.println("s - Start/stop continuous mode");
        Serial.println("h - Show this help");
        break;
        
      default:
        // Clear any other characters
        while (Serial.available() > 0) {
          Serial.read();
        }
        break;
    }
  }
}

// Main setup
void setup() {
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println("\nFreeRTOS Critical Section Interrupt Demo");
  Serial.println("========================================");
  Serial.println("This demo shows safe interrupt handling using critical sections");
  Serial.println("\nCommands:");
  Serial.println("i - Trigger interrupt (increments counter)");
  Serial.println("r - Read counter value");
  Serial.println("s - Start/stop continuous counter updates");
  Serial.println("h - Show help");
  Serial.println("\nType a command and press Enter:");

  // Create counter task
  xTaskCreatePinnedToCore(
    counterTask,
    "Counter Task",
    2048,
    NULL,
    2,            // Higher priority
    &counterTaskHandle,
    app_cpu
  );

  // Create display task
  xTaskCreatePinnedToCore(
    displayTask,
    "Display Task",
    2048,
    NULL,
    1,            // Lower priority
    &displayTaskHandle,
    app_cpu
  );

  // Delete setup task
  vTaskDelete(NULL);
}

void loop() {
  processSerialInput();
  vTaskDelay(pdMS_TO_TICKS(10)); // Small delay for responsiveness
}
