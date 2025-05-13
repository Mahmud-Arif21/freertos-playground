/**
 * FreeRTOS Race Condition Fix with Binary Semaphore
 * 
 * Safely increment a shared variable using a binary semaphore (like a signal)
 */

// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// Global variables
static int shared_var = 0;           // Our shared value
SemaphoreHandle_t bin_sem;           // Binary semaphore

//*****************************************************************************
// Tasks

// Task: Safely increment shared variable (with binary semaphore)
void incTask(void *parameters) {

  int local_var;

  while (1) {                        // Run forever
    // --- CRITICAL SECTION START ---
    // Wait (take) the binary semaphore before accessing shared_var
    if (xSemaphoreTake(bin_sem, portMAX_DELAY) == pdTRUE) {

      // Read shared value (critical section)
      local_var = shared_var;

      // Fake work (makes race condition more likely)     
      vTaskDelay(random(100, 500) / portTICK_PERIOD_MS);
      
      // Modify value      
      local_var++;
      
      // Write back to shared variable      
      shared_var = local_var;

      // Release the semaphore
      xSemaphoreGive(bin_sem);
    }
    // --- CRITICAL SECTION END ---
    
    // Print result (not protected by mutex - okay for demo)
    Serial.println(shared_var);
  }
}

void setup() {

  // Hack to kinda get randomness
  randomSeed(esp_random());

  // Configure Serial
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);  // Wait for serial
  Serial.println("\nBinary Semaphore Demo Starting");
  
  // Create binary semaphore
  bin_sem = xSemaphoreCreateBinary();

  // Make sure it was created successfully
  if (bin_sem == NULL) {
    Serial.println("Failed to create semaphore.");
    while (1);  // Hang if we can't create semaphore
  }

  // Give the semaphore initially so it can be taken
  xSemaphoreGive(bin_sem);

  // Create two identical tasks (they'll compete)
  // Start task 1
  xTaskCreatePinnedToCore(
    incTask,              // Function to run
    "Increment Task 1",   // Task name
    1024,                 // Stack size
    NULL,                 // Parameters
    1,                    // Priority
    NULL,                 // Task handle
    app_cpu               // Core
    );

  // Start task 2
  xTaskCreatePinnedToCore(
    incTask,
    "Increment Task 2",
    1024,
    NULL,
    1,
    NULL,
    app_cpu
    );           

  // Delete "setup and loop" task
  vTaskDelete(NULL);
}

void loop() {
  // Empty loop - all the work is done in tasks
}
