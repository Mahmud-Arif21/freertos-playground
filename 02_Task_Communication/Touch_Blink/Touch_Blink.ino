/**
 * @brief Example code for capacitive touch detection on ESP32 using Touch Pad 0 (T0/GPIO4).
 *
 * **Hardware Setup Recommendation**:
 * - Connect a jumper wire to **GPIO 4 (T0)** to act as an extended touch sensor.  
 *   A longer wire/pad increases surface area for better capacitance change detection.
 * - For stability, add a **10MΩ pull-up/down resistor** between GPIO4 and GND/VCC to reduce noise.
 * - Alternatively, use a dedicated touch-sensitive pad or metal surface for professional applications.
 *
 * This code toggles an LED based on touch detection on GPIO4. Adjust delay or sensitivity in `loop()` as needed.
 * ESP32 touch pads (T0-T9) are sensitive to environmental changes (humidity, nearby objects), so hardware/software tuning may be required.
 */
// Define onboard LED and touch sensor pins
#define LED_PIN 2
#define TOUCH_PIN T0  // GPIO4 (Touch Sensor 0)

/**
 * Task handles for FreeRTOS tasks:
 * - blinkTaskHandle: Controls LED blinking
 * - touchTaskHandle: Manages touch input
 */
TaskHandle_t blinkTaskHandle = NULL;
TaskHandle_t touchTaskHandle = NULL;

/**
 * @brief Blink Task: Toggles LED at fixed intervals when not paused
 *
 * - Runs on Core 0
 * - Toggles LED every 500ms (1Hz frequency)
 * - Can be suspended/resumed by touch input
 * - Maintains LED state when paused
 * 
 * Debug Messages:
 * "Blink task suspended!"
 * "Blink task resumed!"
 */
void blinkTask(void *pvParameters) {
  int core = xPortGetCoreID();
  bool ledState = false;
  pinMode(LED_PIN, OUTPUT);
  
  for(;;) {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
    Serial.printf("LED Is %s [Core %d]\n", ledState ? "ON" : "OFF", core); 
    
    // Delay checks task suspension internally
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

/**
 * @brief Touch Task: Detects capacitive touches and controls blinking
 *
 * - Runs on Core 1
 * - Monitors touch sensor with software debouncing
 * - Toggles blink task state on each valid touch
 * - Prints touch values and system state to serial
 * 
 * Debug Messages:
 * "Touch detected! Value: X - Toggling blink state"
 * "Current state: Blinking PAUSED | Blinking ACTIVE"
 */
void touchTask(void *pvParameters) {
  const int touchThreshold = 20;  // Adjust based on your environment
  bool lastTouchState = false;
  
  for(;;) {
    int core = xPortGetCoreID();
    int touchValue = touchRead(TOUCH_PIN);
    bool currentTouchState = (touchValue < touchThreshold);

    // Detect touch press (falling edge)
    if(currentTouchState && !lastTouchState) {
      Serial.printf("Touch detected! Value: %d - Toggling blink state [Core %d]\n", touchValue, core);
      
      // Toggle blink task state
      if(eTaskGetState(blinkTaskHandle) == eSuspended) {
        vTaskResume(blinkTaskHandle);
        Serial.println("Current state: Blinking ACTIVE");
      } else {
        vTaskSuspend(blinkTaskHandle);
        Serial.println("Current state: Blinking PAUSED");
      }

      // Wait for touch release (rising edge)
      while(touchRead(TOUCH_PIN) < touchThreshold) {
        vTaskDelay(10 / portTICK_PERIOD_MS);
      }
    }

    lastTouchState = currentTouchState;
    vTaskDelay(10 / portTICK_PERIOD_MS);  // 10ms polling interval
  }
}

/**
 * @brief Setup initializes hardware and creates FreeRTOS tasks
 *
 * - Serial communication for debugging
 * - Creates two tasks pinned to different cores
 * - Blink task: Core 0 with priority 1
 * - Touch task: Core 1 with priority 2 (higher priority for responsive input)
 */
void setup() {
  Serial.begin(115200);
  
  // Create blink task on core 0
  xTaskCreatePinnedToCore(
    blinkTask,           // Task function
    "Blink Controller",  // Task name
    2048,                // Stack size
    NULL,                // Parameters
    1,                   // Priority
    &blinkTaskHandle,    // Task handle
    0                    // Core 0
  );

  // Create touch task on core 1
  xTaskCreatePinnedToCore(
    touchTask,           // Task function
    "Touch Controller",  // Task name
    4096,                // Stack size (needs more for touch functions)
    NULL,                // Parameters
    2,                   // Higher priority
    &touchTaskHandle,    // Task handle
    1                    // Core 1
  );

  Serial.println("System initialized - ready for touch input!");
}

void loop() {
  // Not used - FreeRTOS scheduler handles tasks
  vTaskDelete(NULL);
}
