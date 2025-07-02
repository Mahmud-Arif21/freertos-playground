/**
 * FreeRTOS Touch-Controlled LED Timer Demo
 * 
 * Uses a timer to blink an LED on GPIO 2, toggled by touch on GPIO 4 (T0).
 */

// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// Pin definitions
#define LED_PIN 2        // Built-in LED on GPIO2
#define TOUCH_PIN T0     // Capacitive touch pin on GPIO4

// Global variables
static TimerHandle_t blinkTimer = NULL;
static const int TOUCH_THRESHOLD = 40; // Touch detection threshold

//*****************************************************************************
// Timer Callback

// Callback: Toggles LED every 500 ms
void blinkCallback(TimerHandle_t xTimer) {
  static bool ledState = false; // Track LED state
  ledState = !ledState;         // Toggle state
  digitalWrite(LED_PIN, ledState ? HIGH : LOW);
  Serial.println(ledState ? "LED ON" : "LED OFF");
}

//*****************************************************************************
// Tasks

// Task: Monitors touch sensor, toggles timer
void touchTask(void* p) {
  bool wasTouched = false; // Previous touch state

  while (1) {
    int touchValue = touchRead(TOUCH_PIN); // Read touch sensor
    bool isTouched = (touchValue < TOUCH_THRESHOLD); // Detect touch
    // Uncomment to calibrate threshold
    // Serial.println(touchValue);

    // Detect touch on rising edge
    if (isTouched && !wasTouched) {
      if (xTimerIsTimerActive(blinkTimer) == pdFALSE) {
        xTimerStart(blinkTimer, 0); // Start timer
        Serial.println("Timer started - LED blinking");
      } else {
        xTimerStop(blinkTimer, 0);  // Stop timer
        digitalWrite(LED_PIN, LOW); // Ensure LED off
        Serial.println("Timer stopped - LED off");
      }
      vTaskDelay(pdMS_TO_TICKS(50)); // Debounce delay
    }
    wasTouched = isTouched; // Update state
    vTaskDelay(pdMS_TO_TICKS(10)); // Polling interval
  }
}

//*****************************************************************************
// Main

void setup() {
  // Configure Serial
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS); // Wait for serial
  Serial.println("\nTouch-Controlled LED Timer Demo Starting");

  // Configure LED pin
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Create timer
  blinkTimer = xTimerCreate("BlinkTimer", pdMS_TO_TICKS(500), pdTRUE, NULL, blinkCallback);

  // Check timer creation
  if (blinkTimer == NULL) {
    Serial.println("Failed to create timer");
    while (1); // Halt if timer creation fails
  }

  // Create touch task, pinned to core 1
  xTaskCreatePinnedToCore(
    touchTask,     // Function to run
    "Touch Task",  // Task name
    1024,          // Stack size
    NULL,          // Parameters
    1,             // Priority
    NULL,          // Task handle
    app_cpu        // Core
  );

  // Delete setup and loop task
  vTaskDelete(NULL);
}

void loop() {
  // Empty loop - all work done in tasks and timers
}