
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
 * @brief Initializes system components and launches FreeRTOS task
 *
 * This setup function performs critical initialization steps:
 * - Enables high-speed serial communication (115200 baud)
 * - Configures LED pin as digital output and turns off LED at startup
 * - Creates and pins a FreeRTOS task to core 1 for LED control
 *
 * @details 
 * - Points to the function that will be executed as a task (ledTask)
 * - Must have the signature void functionName(void *parameter)
 * - A descriptive string "LED Task" used for debugging purposes
 * - Allocates 2048 bytes of memory for this task's stack (local variables, function call information, return addresses)
 * - 2048 bytes is a moderate size, suitable for tasks with moderate complexity
 * - If a task exceeds its stack, it will cause a stack overflow (crash)
 * - NULL means no parameters are being passed
 * - Sets the priority from 1 (lowest) to 24 (highest)
 * - Passing the address of 'ledTaskHandle' variable declared at the beginning as (&ledTaskHandle)
 * - Core 0 typically handles WiFi/Bluetooth and the Arduino's loop() function
 * - Core 1 is reserved for this task (used for application-specific tasks)
 * 
 */
void setup() {
  Serial.begin(115200);           // Baud Rate
  pinMode(LED_PIN, OUTPUT);       // Pin Configuration
  digitalWrite(LED_PIN, LOW);     // Turn Off LED At Startup

  xTaskCreatePinnedToCore(        // xTaskCreatePinnedToCore(task function, "task name", stack size in kb,
                                  //  task parameter, priority, specific task handle variable, core ID);
                                  
    ledTask,          // Task Function
    "LED Task",       // Task Name (for debugging purpose)
    2048,             // Stack Size
    NULL,             // Task Parameter
    1,                // Task Priority (1-24)
    &ledTaskHandle,   // Task Handle
    1                 // Core Number To Run The Task On

    );
}

void loop() {
  // Empty loop - all the work is done in the task

}
