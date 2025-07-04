/**
 * FreeFTPOS Multiple Touch Interrupt Demo
 * 
 * Different touch sensors trigger unique LED patterns:
 * - T0 (GPIO 4): Rapid blink pattern
 * - T1 (GPIO 0): Slow pulse pattern  
 * - T2 (GPIO 2): SOS pattern
 */

#include <Arduino.h>
#include <freertos/FreeRTOS.h>

// Use core 0 for demo purposes
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 0;
#endif

// Pin definitions
#define LED_PIN 2       // Built-in LED on GPIO 2
#define TOUCH_PIN1 T0   // GPIO 4
#define TOUCH_PIN2 T1   // GPIO 0
#define TOUCH_PIN3 T2   // GPIO 2 (same as LED, but different function)

// Global variables
static TaskHandle_t ledTaskHandle = NULL;
static const int TOUCH_THRESHOLD = 40;

// Touch pattern identifiers
#define PATTERN_RAPID 1
#define PATTERN_PULSE 2
#define PATTERN_SOS 3

// ISR: T0 - Rapid blink pattern
void IRAM_ATTR touch1ISR() {
  BaseType_t higherPriorityTaskWoken = pdFALSE;
  xTaskNotifyFromISR(ledTaskHandle, PATTERN_RAPID, eSetValueWithOverwrite, &higherPriorityTaskWoken);
  portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

// ISR: T1 - Slow pulse pattern
void IRAM_ATTR touch2ISR() {
  BaseType_t higherPriorityTaskWoken = pdFALSE;
  xTaskNotifyFromISR(ledTaskHandle, PATTERN_PULSE, eSetValueWithOverwrite, &higherPriorityTaskWoken);
  portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

// ISR: T2 - SOS pattern
void IRAM_ATTR touch3ISR() {
  BaseType_t higherPriorityTaskWoken = pdFALSE;
  xTaskNotifyFromISR(ledTaskHandle, PATTERN_SOS, eSetValueWithOverwrite, &higherPriorityTaskWoken);
  portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

// LED pattern functions
void rapidBlink() {
  Serial.println("Rapid blink pattern");
  for (int i = 0; i < 10; i++) {
    digitalWrite(LED_PIN, HIGH);
    vTaskDelay(50 / portTICK_PERIOD_MS);
    digitalWrite(LED_PIN, LOW);
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

void slowPulse() {
  Serial.println("Slow pulse pattern");
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    vTaskDelay(800 / portTICK_PERIOD_MS);
    digitalWrite(LED_PIN, LOW);
    vTaskDelay(800 / portTICK_PERIOD_MS);
  }
}

void sosPattern() {
  Serial.println("SOS pattern");
  // S: 3 short
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    vTaskDelay(150 / portTICK_PERIOD_MS);
    digitalWrite(LED_PIN, LOW);
    vTaskDelay(150 / portTICK_PERIOD_MS);
  }
  vTaskDelay(200 / portTICK_PERIOD_MS);
  
  // O: 3 long
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    digitalWrite(LED_PIN, LOW);
    vTaskDelay(150 / portTICK_PERIOD_MS);
  }
  vTaskDelay(200 / portTICK_PERIOD_MS);
  
  // S: 3 short
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    vTaskDelay(150 / portTICK_PERIOD_MS);
    digitalWrite(LED_PIN, LOW);
    vTaskDelay(150 / portTICK_PERIOD_MS);
  }
}

// Task: Executes LED pattern based on touch sensor
void ledTask(void* p) {
  while (1) {
    uint32_t pattern = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    
    switch(pattern) {
      case PATTERN_RAPID:
        rapidBlink();
        break;
      case PATTERN_PULSE:
        slowPulse();
        break;
      case PATTERN_SOS:
        sosPattern();
        break;
      default:
        Serial.println("Unknown pattern");
        break;
    }
    
    // Ensure LED is off after pattern
    digitalWrite(LED_PIN, LOW);
  }
}

// Main
void setup() {
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println("\nMultiple Touch Interrupt Demo Starting");
  Serial.println("Touch sensors:");
  Serial.println("- T0 (GPIO 4): Rapid blink");
  Serial.println("- T1 (GPIO 0): Slow pulse");
  Serial.println("- T2 (GPIO 2): SOS pattern");

  // Configure LED pin
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Create task
  xTaskCreatePinnedToCore(
    ledTask,
    "LED Task",
    2048,         // Increased stack size for patterns
    NULL,
    1,
    &ledTaskHandle,
    app_cpu
  );

  // Attach touch interrupts
  touchAttachInterrupt(TOUCH_PIN1, touch1ISR, TOUCH_THRESHOLD);
  touchAttachInterrupt(TOUCH_PIN2, touch2ISR, TOUCH_THRESHOLD);
  touchAttachInterrupt(TOUCH_PIN3, touch3ISR, TOUCH_THRESHOLD);

  // Delete setup task
  vTaskDelete(NULL);
}

void loop() {
  // Empty - all work done in tasks and ISRs
}