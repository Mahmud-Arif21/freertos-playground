# Lesson01: LED Blink

## Description

This lesson demonstrates how to blink an LED using FreeRTOS on an Arduino-compatible board. It focuses on the code structure and usage of FreeRTOS tasks to replace the standard `loop()` function.

## Code Overview

- **`setup()`**: Configures the LED pin as an output and starts the FreeRTOS scheduler by creating the blink task.
- **`loop()`**: Left empty because the FreeRTOS scheduler manages task execution.
- **`blinkTask(void* parameters)`**: A FreeRTOS task that toggles the LED state and uses `vTaskDelay()` to wait 500 ms between toggles.

## Key Code Snippets and Explanations

### 1. LED Pin Definition

```cpp
// Define onboard LED pin (digital pin 2)
#define LED_PIN 2
```

This macro names the physical pin connected to the LED, making it easy to change in one place if your hardware wiring differs.

---

### 2. Task Handle Declaration

```cpp
// Handle used to manage the LED blink task
TaskHandle_t ledTaskHandle = NULL;
```

The `TaskHandle_t` variable starts as `NULL` and is populated when the blink task is created. It allows you to suspend, resume, or delete the task at runtime.

---

### 3. Setup and Task Creation

```cpp
void setup() {
    pinMode(LED_PIN, OUTPUT);
    xTaskCreate(
        blinkTask,      // Function to run as a task
        "LED Task",    // Name visible in debugger
        2048,           // Stack size in bytes
        NULL,           // No parameters passed
        1,              // Priority (1 = low, higher = more urgent)
        &ledTaskHandle  // Receives the task handle
    );
    vTaskStartScheduler();  // Begin FreeRTOS task scheduling
}
```

Here, we configure the LED pin, create the FreeRTOS task with a stack of 2 KB and low priority, then start the scheduler to manage all tasks instead of `loop()`.

---

### 4. Blink Task Implementation

```cpp
void blinkTask(void* parameters) {
    (void) parameters;   // Unused parameter
    for (;;) {
        digitalWrite(LED_PIN, HIGH);
        vTaskDelay(pdMS_TO_TICKS(500));  // Delay for 500 ms without blocking other tasks
        digitalWrite(LED_PIN, LOW);
        vTaskDelay(pdMS_TO_TICKS(500));  // Delay for 500 ms
    }
}
```

This infinite loop toggles the LED on and off every 500 ms. Using `vTaskDelay()` yields the CPU to other tasks, demonstrating non-blocking delays.

---

> **Note:** Additional inline details and edge-case comments are included in the `01_LED_Blink.ino`. Feel free to explore the code comments for deeper insights.

## How to Run

1. Open `01_LED_Blink.ino` in the Arduino IDE.
2. Choose your board type and the correct serial port.
3. Click **Upload** to program your board.
4. After uploading, the LED connected to the specified pin will start blinking at 1 Hz.

## Learning Objectives

- Create and configure FreeRTOS tasks on Arduino.
- Use `vTaskDelay()` for non-blocking delays in tasks.
- Understand how FreeRTOS scheduler replaces the standard `loop()` function.
