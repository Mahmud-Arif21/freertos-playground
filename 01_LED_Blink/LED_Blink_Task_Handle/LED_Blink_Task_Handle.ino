// Define onboard LED > pin-2
#define LED_PIN 2

/**
 * Creates a task handle variable to manage the `ledTask` task
 * - Used to control task lifecycle (suspend/resume/delete)
 * - Required for multi-core operations (pinning to specific cores)
 * - Initialized to NULL to indicate an invalid handle until task creation
 * 
 * Example Usages:
 * 1. Delete task:       vTaskDelete(ledTaskHandle);                    // Permanently delete the task
 * 2. Suspend task:     vTaskSuspend(ledTaskHandle);                    // Pause task execution
 * 3. Resume task:      vTaskResume(ledTaskHandle);                     // Resume paused task
 * 4. Change priority:  vTaskPrioritySet(ledTaskHandle, NEW_PRIORITY);  // Set new priority level (1-24)
 * 5. Query status:     eTaskGetState(ledTaskHandle);                   // Check task state (e.g., Running/Blocked)
 * 
 * Always validate handle before use:
 * if (ledTaskHandle != NULL) {
 *   // Safe to perform task operations
 * }
 */
 TaskHandle_t ledTaskHandle = NULL; 

/**
 * @brief Task Function to toggle onboard LED and log core information
 *
 * This FreeRTOS task:
 * - Toggles the onboard LED (Pin 2 on esp32)
 * - Prints debug messages via Serial showing execution core
 * - Implements a 1-second interval using vTaskDelay
 * - Runs indefinitely until explicitly deleted
 *
 * @param pvParameters Pointer to task-specific parameters (unused in this task)
 * 
 * @note
 * - Core affinity is enforced by xTaskCreatePinnedToCore() in setup()
 * - Requires LED_PIN to be configured as OUTPUT before task starts
 * - Uses blocking delays (vTaskDelay) to allow context switching
 * - Task handle (ledTaskHandle) enables runtime control (suspend/delete/priority change)
 * 
 * Example Output:
 * "LED ON - Running on core 1"
 * "LED OFF - Running on core 1"
 */
void ledTask(void *pvParameters) {
  // Retrive core number the task is running on
  int core = xPortGetCoreID();
  
  for(;;) {
    digitalWrite(LED_PIN, HIGH);
    Serial.printf("LED ON - Running on core %d\n", core);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    
    digitalWrite(LED_PIN, LOW);
    Serial.printf("LED OFF - Running on core %d\n", core);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

/**
 * @brief Task function to handle serial commands for controlling the LED task and direct LED toggling
 *
 * This FreeRTOS task:
 * - Monitors serial input for commands 's', 'r', and 'd'
 * - Suspends or resumes the `ledTask` based on 's' and 'r' commands
 * - Toggles the LED directly when 'd' is received, regardless of `ledTask` state
 * - Prints confirmation messages via Serial
 * - Implements a 100ms polling interval using vTaskDelay
 * - Runs indefinitely on core 0 with a higher priority than `ledTask`
 *
 * @param pvParameters Pointer to task-specific parameters (unused in this task)
 * 
 * @note
 * - Core affinity is enforced by xTaskCreatePinnedToCore() in setup()
 * - Runs at priority 2 to ensure responsiveness to serial input
 * - Direct LED toggling uses digitalWrite/Read, which are thread-safe in ESP32 Arduino core
 * - Task handle is not stored, so lifecycle control is not implemented for this task
 * - Uses non-blocking delays to maintain responsiveness
 * 
 * Example Usage:
 * Send 's' over Serial: Suspends LED blinking
 * Send 'r' over Serial: Resumes LED blinking
 * Send 'd' over Serial: Toggles LED immediately
 */
void serialControlTask(void *parameter) {
  for(;;) {
    if(Serial.available() > 0) {
      char command = Serial.read();

      switch (command) {
        case 's':
          // suspend blink task
          vTaskSuspend(ledTaskHandle);
          Serial.println("Blink Task Suspended");
          break;

        case 'r':
          // Resume blink task
          vTaskResume(ledTaskHandle);
          Serial.println("Blink Task Resumed");
          break;

        case 'd':
          // Toggle LED directly (works even when task is suspended)
          digitalWrite(LED_PIN, !digitalRead(LED_PIN));
          Serial.println("LED toggled directly");
          break;
      }
    }

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

/**
 * @brief Initializes system components and launches FreeRTOS tasks
 *
 * This setup function performs critical initialization steps:
 * - Enables high-speed serial communication (115200 baud)
 * - Configures LED pin as digital output and turns off LED at startup
 * - Creates and pins two FreeRTOS tasks:
 *   1. `ledTask` on core 1 for LED control
 *   2. `serialControlTask` on core 0 for handling serial commands
 *
 * @details 
 * - Both tasks use 2048-byte stacks and have distinct priorities:
 *   - LED Task: Priority 1 (standard)
 *   - Control Task: Priority 2 (higher for input responsiveness)
 * - Core assignments ensure parallel execution:
 *   - Core 0: Handles serial commands and Arduino loop()
 *   - Core 1: Dedicated to LED blinking operations
 * - ledTaskHandle provides explicit control over LED task lifecycle
 * - Serial control task runs indefinitely without a stored handle
 * 
 * Task Creation Parameters:
 * xTaskCreatePinnedToCore(task_function, "name", stack_size, params, priority, handle, core)
 */
void setup() {
  Serial.begin(115200);           // Baud Rate
  pinMode(LED_PIN, OUTPUT);       // Pin Configuration
  digitalWrite(LED_PIN, LOW);     // Turn Off LED At Startup

  xTaskCreatePinnedToCore(        
    ledTask,          // Task Function
    "LED Task",       // Task Name
    2048,             // Stack Size
    NULL,             // Parameters
    1,                // Priority
    &ledTaskHandle,   // Handle
    1                 // Core 1
  );

  xTaskCreatePinnedToCore(        
    serialControlTask,// Task Function
    "Control Task",   // Task Name
    2048,             // Stack Size
    NULL,             // Parameters
    2,                // Higher Priority
    NULL,             // No handle storage
    0                 // Core 0
  );
}

/**
 * @brief Main loop remains empty as tasks handle all functionality
 * 
 * FreeRTOS scheduler manages task execution automatically.
 * No application logic required here due to task-based design.
 */
void loop() {
  // Empty loop - all the work is done in the task
}
