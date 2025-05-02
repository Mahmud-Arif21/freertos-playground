# Lesson 4: Shared Resource Protection with Mutexes

## Introduction

In multitasking systems, when two or more tasks share a resource without coordination, you get a **race condition**—unpredictable behavior that’s hard to debug. A **mutex** (mutual exclusion) is a lightweight lock that ensures only one task at a time can access the critical section, keeping shared data and peripherals safe.

## Learning Objectives

By the end of this lesson, you will:

1. Observe a race condition in action
2. Create and use a mutex in FreeRTOS
3. Protect shared data and peripherals with a mutex
4. See how mutexes prevent tasks from colliding

---

## 1. Race Condition Demo

Let’s first see a race condition example by **Shawn Hymel**. Two tasks increment the same `counter` without any lock:

```cpp
/**
 * FreeRTOS Race Condition Demo
 * 
 * Increment a shared global variable.
 * 
 * Date: January 20, 2021
 * Author: Shawn Hymel
 * License: 0BSD
 */

// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// Globals
static int shared_var = 0;

//*****************************************************************************
// Tasks

// Increment shared variable (the wrong way)
void incTask(void *parameters) {

  int local_var;

  // Loop forever
  while (1) {

    // Roundabout way to do "shared_var++" randomly and poorly
    local_var = shared_var;
    local_var++;
    vTaskDelay(random(100, 500) / portTICK_PERIOD_MS);
    shared_var = local_var;
  
    // Print out new shared variable
    Serial.println(shared_var);
  }
}

//*****************************************************************************
// Main (runs as its own task with priority 1 on core 1)

void setup() {

  // Hack to kinda get randomness
  randomSeed(analogRead(0));

  // Configure Serial
  Serial.begin(115200);

  // Wait a moment to start (so we don't miss Serial output)
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("---FreeRTOS Race Condition Demo---");

  // Start task 1
  xTaskCreatePinnedToCore(incTask,
                          "Increment Task 1",
                          1024,
                          NULL,
                          1,
                          NULL,
                          app_cpu);

  // Start task 2
  xTaskCreatePinnedToCore(incTask,
                          "Increment Task 2",
                          1024,
                          NULL,
                          1,
                          NULL,
                          app_cpu);           

  // Delete "setup and loop" task
  vTaskDelete(NULL);
}

void loop() {
  // Execution should never get here
}
```

**What happens?**

- The tasks both update `counter` at different times.
- In practice, some increments get lost or overlap, and the printed `counter` jumps unpredictably.
- This is the race condition—you cannot trust the result.

---

## 2. Introducing Mutexes

A **mutex** is like a single‑key lock on a shared resource. A task must take the mutex before entering the critical section and give it back when done.

```cpp
// Create a mutex (in setup)
SemaphoreHandle_t myMutex = xSemaphoreCreateMutex();

// Protecting a critical section
if (xSemaphoreTake(myMutex, portMAX_DELAY) == pdTRUE) {
  // critical code here
  xSemaphoreGive(myMutex);
}
```

- `xSemaphoreCreateMutex()` returns a handle used by all tasks.
- `xSemaphoreTake()` waits until the mutex is free, then locks it.
- `xSemaphoreGive()` unlocks it, letting another task in.

---

## 3. Fixing the Race Demo with a Mutex

Here’s a complete solution using a mutex to protect `shared_var`. We pin both tasks to a single core (core 1) and use random delays to simulate work:

```cpp
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
```

## **Result:** With the mutex lock, `shared_var` grows smoothly—no more race glitches.

## 4. LED Blink Example with Mutex

To further illustrate, here’s two tasks blinking the on‑board LED twice each, but never interleaving:

```cpp
#include <Arduino.h>

// ======== Configuration ==========
#define LED_PIN 2

SemaphoreHandle_t ledMutex;         // Our "LED access token"

// ======== LED Blink Task =========
void blinkTask(void *pvParameters) {
  const char* taskName = (const char*) pvParameters;
  
  pinMode(LED_PIN, OUTPUT);         // Setup LED pin
  
  for (;;) {                        // Infinite loop
    // --- GET LED ACCESS ---
    xSemaphoreTake(ledMutex, portMAX_DELAY);  // Wait forever for mutex
    
    // --- CRITICAL SECTION START ---
    Serial.printf("%s: Started blinking\n", taskName);
    
    // Blink pattern (protected from interference)
    for (int i = 0; i < 2; i++) {
      digitalWrite(LED_PIN, HIGH);
      vTaskDelay(200 / portTICK_PERIOD_MS);  // LED on 200ms
      digitalWrite(LED_PIN, LOW);
      vTaskDelay(200 / portTICK_PERIOD_MS);  // LED off 200ms
    }
    // --- CRITICAL SECTION END ---
    
    xSemaphoreGive(ledMutex);       // Release mutex
    
    // Wait before next blink attempt
    vTaskDelay(1000 / portTICK_PERIOD_MS);  // 1 second pause
  }
}

// ======== Main Setup =============
void setup() {
  Serial.begin(115200);
  
  vTaskDelay(1000 / portTICK_PERIOD_MS);  // Wait for serial
  Serial.println("\nLED Mutex Demo Start");

  // Create our mutex (like making a key)
  ledMutex = xSemaphoreCreateMutex();
  
  if (ledMutex == NULL) {
    Serial.println("Failed to create mutex!");
    while(1);  // Freeze if mutex fails
  }

  // Create two competing tasks
  xTaskCreatePinnedToCore(
    blinkTask,    // Task function
    "BlinkA",     // Task name
    1024,         // Stack size
    (void*)"TaskA",  // Parameter (text)
    1,            // Priority
    NULL,         // Task handle
    0             // Core 0
  );

  xTaskCreatePinnedToCore(
    blinkTask,
    "BlinkB",
    1024,
    (void*)"TaskB",
    1,
    NULL,
    1             // Core 1
  );
}

void loop() {
  // Execution should never get here
}
```

Now, each task holds the lock, blinks twice, then releases it—so you always see two clean blinks from one task before the other runs.

---

## Key Points

- A **race condition** happens when multiple tasks touch shared data or hardware at the same time.
- A **mutex** enforces exclusive access—only one task in the critical section.
- Always pair each `xSemaphoreTake()` with a `xSemaphoreGive()`.


