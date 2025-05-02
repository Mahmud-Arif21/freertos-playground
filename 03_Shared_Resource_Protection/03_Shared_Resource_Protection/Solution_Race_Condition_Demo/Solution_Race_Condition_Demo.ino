/**
 * FreeRTOS Race Condition Fix with Mutex
 * 
 * Safely increment a shared variable using a mutex (like a lock)
 */

// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// Global variables
static int shared_var = 0;           // Our dangerous shared value
static SemaphoreHandle_t mutex;      // Our protection lock

// Task: Safely increment shared variable
void incTask(void *parameters) {
  int local_var;
  
  while (1) {                        // Run forever
    // --- CRITICAL SECTION START ---
    // Get the lock (wait forever if needed)
    if (xSemaphoreTake(mutex, portMAX_DELAY)) {
      
      // Read shared value
      local_var = shared_var;
      
      // Fake work (makes race condition more likely)
      vTaskDelay(random(100, 500) / portTICK_PERIOD_MS);
      
      // Modify value
      local_var++;
      
      // Write back to shared variable
      shared_var = local_var;
      
      // Release the lock
      xSemaphoreGive(mutex);
    }
    // --- CRITICAL SECTION END ---
    
    // Print result (not protected by mutex - okay for demo)
    Serial.println(shared_var);
  }
}

void setup() {
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);  // Wait for serial
  Serial.println("\nMutex Demo Starting");

  // Creating the mutex (our key) first
  mutex = xSemaphoreCreateMutex();
  if (mutex == NULL) {
    Serial.println("Failed to create mutex!");
    while(1);  // Hang if we can't create mutex
  }

  // Create two identical tasks (they'll compete)
  xTaskCreatePinnedToCore(
    incTask,    // Function to run
    "Task 1",   // Task name
    1024,       // Stack size
    NULL,       // Parameters
    1,          // Priority
    NULL,       // Task handle
    app_cpu     // Core
  );

  xTaskCreatePinnedToCore(
    incTask,
    "Task 2",
    1024,
    NULL,
    1,
    NULL,
    app_cpu
  );

  vTaskDelete(NULL);  // Delete setup task
}

void loop() {
    // Empty loop - all the work is done in tasks
}
