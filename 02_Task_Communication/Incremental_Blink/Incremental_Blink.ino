// Define onboard LED > pin-2
#define LED_PIN 2

/**
 * Creates task handle variables to manage the FreeRTOS tasks:
 * - senderTaskHandle: Manages the task that sends integers to a queue
 * - receiverTaskHandle: Manages the task that reads from the queue and blinks the LED
 * 
 * These handles allow you to:
 * 1. Delete task:       vTaskDelete(taskHandle);
 * 2. Suspend task:      vTaskSuspend(taskHandle);
 * 3. Resume task:       vTaskResume(taskHandle);
 * 4. Change priority:   vTaskPrioritySet(taskHandle, NEW_PRIORITY);
 * 5. Query status:      eTaskGetState(taskHandle);
 * 
 * Always check if handle != NULL before using.
 */
 TaskHandle_t senderTaskHandle = NULL;
 TaskHandle_t receiverTaskHandle = NULL;

/**
 * Global queue handle for integer communication between tasks
 * - numberQueue will store integer values sent by senderTask
 * - receiverTask will read these integers and blink LED accordingly
 * - Queue length: 5, Item size: sizeof(int)
 */
 QueueHandle_t  numberQueue;

/**
 * @brief Sender Task: Sends integer values (1 to 5) to the queue in a loop
 *
 * - Executes on Core 0
 * - Increments an integer and sends it to the queue every 3 seconds
 * - If queue is full, it prints a warning
 * - Resets to 1 after sending 5
 * 
 * Debug Example:
 * "Sender [Core 0]: Sent value 3 to queue"
 * "Sender [Core 0]: Failed to send to queue"
 *
 * @param pvParameters Unused (NULL)
 */
void senderTask(void *pvParameters) {
  int counter = 1; // Start with 1
  
  for(;;) {
    // Get the core ID for debug output
    int core = xPortGetCoreID();

    // Send the counter value to the queue
    if(xQueueSend(numberQueue, &counter, 1000 / portTICK_PERIOD_MS) == pdTRUE) {
      // Successfully sent to queue
      Serial.printf("Sender [Core %d]: Sent value %d to queue\n", core, counter);

      // Increment counter (1, 2, 3, 4, 5, 1, 2, 3, 4, 5, ...)
      counter++;
      
      if(counter > 5) counter = 1; // Reset after reaching 5
    }

    else {
      // Failed to send (queue might be full)
      Serial.printf("Sender [Core %d]: Failed to send to queue\n", core);
    }

    // Wait for 3 seconds befor sending the next value
    vTaskDelay(3000 / portTICK_PERIOD_MS);
  }
}

/**
 * @brief Receiver Task: Reads integers from queue and blinks LED accordingly
 *
 * - Executes on Core 1
 * - Waits up to 5 seconds to receive a value from queue
 * - Blinks onboard LED based on received value (e.g., blink 3 times for value 3)
 * - If no value is received in time, logs a timeout
 * 
 * Debug Example:
 * "Receiver [Core 1]: Got value 2 from queue"
 * "Receiver [Core 1]: No value received (timeout)"
 *
 * @param pvParameters Unused (NULL)
 */
 void receiverTask(void *pvParameters) {
  int receivedValue;
  
  for(;;) {
    // Get core ID
    int core = xPortGetCoreID();

    // Try to receive an item from the queue (wait for up to 5 seconds)
    if(xQueueReceive(numberQueue, &receivedValue, 5000 / portTICK_PERIOD_MS) == pdTRUE) {
      // Successfully received from queue
      Serial.printf("Receiver [Core %d]: Got value %d from queue\n", core, receivedValue);

      // Blink the LED the number of times indicated by the received value
      for(int i = 0; i < receivedValue; i++) {
        digitalWrite(LED_PIN, HIGH);
        vTaskDelay(200 / portTICK_PERIOD_MS); // 200 ms
        digitalWrite(LED_PIN, LOW);
        vTaskDelay(200 / portTICK_PERIOD_MS);
      }

      // Longer pause after completing the blink sequence
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    else {
      // Timeout occurred
      Serial.printf("Receiver [Core %d]: No value received (timeout)\n", core);
    }
  }
}

/**
 * @brief Sets up Serial, LED pin, queue, and creates tasks on separate cores
 *
 * - Initializes Serial communication at 115200 baud
 * - Configures built-in LED pin as OUTPUT and turns it off
 * - Creates a queue to transfer integer values between tasks
 * - Starts two FreeRTOS tasks:
 *    - senderTask on Core 0: sends values to the queue
 *    - receiverTask on Core 1: receives values and blinks LED
 *
 * Memory Notes:
 * - Stack size for each task is 2048 bytes (adequate for basic logic)
 * - NULL task parameters passed since no task-specific arguments needed
 * - Priority set to 1 (lowest usable priority)
 */
void setup() {
  Serial.begin(115200);           // Baud Rate
  pinMode(LED_PIN, OUTPUT);       // Pin Configuration
  digitalWrite(LED_PIN, LOW);     // Turn Off LED At Startup

  // Create a queue that can hold up to 5 integers
  numberQueue = xQueueCreate(5, sizeof(int));

  if(numberQueue == NULL) {
    Serial.println("Error: Could not create queue");
    while(1); // Stop if queue creation failed
  }

  // Create sender task on core 0
  xTaskCreatePinnedToCore(
    
    senderTask,          // Task Function
    "Sender Task",       // Task Name (for debugging purpose)
    2048,                // Stack Size
    NULL,                // Task Parameter
    1,                   // Task Priority (1-24 => low-high)
    &senderTaskHandle,   // Task Handle
    0                    // Core Number To Run The Task On (0)

    );

  // Create receiver task on core 1
  xTaskCreatePinnedToCore(
    
    receiverTask,        // Task Function
    "Receiver Task",     // Task Name (for debugging purpose)
    2048,                // Stack Size
    NULL,                // Task Parameter
    1,                   // Task Priority (1-24 => low-high)
    &receiverTaskHandle, // Task Handle
    1                    // Core Number To Run The Task On (1)

    );
  
  Serial.println("All tasks created!");
}

void loop() {
  // Empty loop - all the work is done in the task
  vTaskDelay(1000 / portTICK_PERIOD_MS);

}
