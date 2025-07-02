/**
 * FreeRTOS Software Timer Demo
 * 
 * Demonstrates one-shot and auto-reload timers logging to Serial.
 */

// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// Global variables
static TimerHandle_t autoReloadTimer = NULL;
static TimerHandle_t oneShotTimer = NULL;

//*****************************************************************************
// Timer Callbacks

// Callback: Auto-reload timer (every 2 seconds)
void autoReloadCallback(TimerHandle_t xTimer) {
  Serial.println("Auto-reload timer triggered");
}

// Callback: One-shot timer (once after 5 seconds)
void oneShotCallback(TimerHandle_t xTimer) {
  Serial.println("One-shot timer triggered");
}

//*****************************************************************************
// Main

void setup() {
  // Configure Serial
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS); // Wait for serial
  Serial.println("\nSoftware Timer Demo Starting");

  // Create timers
  autoReloadTimer = xTimerCreate("AutoReload", pdMS_TO_TICKS(2000), pdTRUE, NULL, autoReloadCallback);
  oneShotTimer = xTimerCreate("OneShot", pdMS_TO_TICKS(5000), pdFALSE, NULL, oneShotCallback);

  // Check timer creation
  if (autoReloadTimer == NULL || oneShotTimer == NULL) {
    Serial.println("Failed to create timers");
    while (1); // Halt if timer creation fails
  }

  // Start timers
  xTimerStart(autoReloadTimer, 0);
  xTimerStart(oneShotTimer, 0);

  // Delete setup and loop task
  vTaskDelete(NULL);
}

void loop() {
  // Empty loop - all work done in timers
}